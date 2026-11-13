// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

// Automated echo integration test (single process):
//   - one CppNet server instance echoing on 127.0.0.1:19021
//   - one CppNet client instance opening 8 connections
//   - each connection sends payloads of 1K/4K/8K/16K/64K bytes and the
//     received stream is verified byte by byte against the expected
//     pattern.
//
// Regression intent: payload sizes that are multiples of the 4K read
// buffer expansion step used to hit the EAGAIN path at the end of
// RWSocket::Recv, which rolled the buffer write point back by one byte
// and silently dropped the last received byte. The 4K/8K/16K/64K sizes
// keep that code path under test.
//
// The binary is self terminating: 30s watchdog, exit code 0 on success,
// 1 on any failure (connect error, disconnect before completion, byte
// mismatch, timeout). It never tears the library down gracefully, it
// _Exit()s while the IO threads are parked: this avoids static
// destruction racing live callbacks in a test harness.

#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>

#include "include/cppnet.h"

using namespace cppnet;

namespace {

const uint16_t    kPort      = 19021;   // 8921 is used by the echo demos
const int         kConnCount = 8;
const uint32_t    kSizes[]   = {1024, 4096, 8192, 16384, 65536};

std::vector<uint8_t> g_expected;                 // full per-connection stream
std::unordered_map<uint64_t, uint64_t> g_progress; // fd -> verified bytes
std::mutex      g_mutex;
std::atomic<int> g_conn_done(0);
std::atomic<bool> g_verified(false);
std::atomic<bool> g_failed(false);

void BuildExpected() {
    uint32_t total = 0;
    for (uint32_t s : kSizes) {
        total += s;
    }
    g_expected.reserve(total);
    for (uint32_t s : kSizes) {
        for (uint32_t j = 0; j < s; j++) {
            g_expected.push_back(static_cast<uint8_t>((j * 131 + s * 7 + 11) & 0xFF));
        }
    }
}

void Fail(const char* stage, const char* detail) {
    // best effort diagnostics from any thread
    fprintf(stderr, "FAIL [%s] %s\n", stage, detail ? detail : "");
    fflush(stderr);
    g_failed = true;
    std::_Exit(1);
}

// --------------------------------------------------------------- server ---

void ServerAcceptFunc(Handle handle, uint32_t error) {
    if (error != CEC_SUCCESS) {
        Fail("server-accept", "accept/connect callback got error");
    }
}

void ServerReadFunc(Handle handle, cppnet::BufferPtr data, uint32_t len) {
    (void)len;
    // echo everything that is currently readable, chunk by chunk
    while (data->GetCanReadLength() > 0) {
        char buf[16 * 1024];
        uint32_t r = data->Read(buf, sizeof(buf));
        if (r == 0) {
            break;
        }
        if (!handle->Write(buf, r)) {
            Fail("server-echo", "handle->Write refused (write cache over limit?)");
        }
    }
}

void ServerDisconnectFunc(Handle handle, uint32_t err) {
    (void)handle;
    (void)err;
    if (!g_verified) {
        Fail("server-disconnect", "peer closed before all data was echoed");
    }
}

// --------------------------------------------------------------- client ---

void ClientConnectFunc(Handle handle, uint32_t error) {
    if (error != CEC_SUCCESS) {
        Fail("client-connect", "connection failed (is the test port busy?)");
    }
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        g_progress[handle->GetSocket()] = 0;
    }
    // queue the full per-connection stream: 1K + 4K + 8K + 16K + 64K
    uint32_t off = 0;
    for (uint32_t s : kSizes) {
        if (!handle->Write(reinterpret_cast<const char*>(&g_expected[off]), s)) {
            Fail("client-send", "handle->Write refused while queueing payload");
        }
        off += s;
    }
}

void ClientReadFunc(Handle handle, cppnet::BufferPtr data, uint32_t len) {
    (void)len;
    uint64_t verified = 0;
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        verified = g_progress[handle->GetSocket()];
    }

    while (data->GetCanReadLength() > 0) {
        char buf[16 * 1024];
        uint32_t r = data->Read(buf, sizeof(buf));
        if (r == 0) {
            break;
        }
        if (verified + r > g_expected.size()) {
            Fail("client-verify", "received more bytes than were sent");
        }
        for (uint32_t i = 0; i < r; i++) {
            if (static_cast<uint8_t>(buf[i]) != g_expected[verified + i]) {
                char detail[128];
                snprintf(detail, sizeof(detail),
                         "byte mismatch on socket %llu at stream offset %llu: "
                         "want 0x%02x got 0x%02x",
                         (unsigned long long)handle->GetSocket(),
                         (unsigned long long)(verified + i),
                         g_expected[verified + i], (uint8_t)buf[i]);
                Fail("client-verify", detail);
            }
        }
        verified += r;
    }

    {
        std::lock_guard<std::mutex> lock(g_mutex);
        g_progress[handle->GetSocket()] = verified;
    }

    if (verified == g_expected.size() && g_conn_done.fetch_add(1) + 1 == kConnCount) {
        g_verified = true;
    }
}

void ClientDisconnectFunc(Handle handle, uint32_t err) {
    (void)handle;
    (void)err;
    if (!g_verified) {
        Fail("client-disconnect", "connection lost before the stream was verified");
    }
}

// ------------------------------------------------------------- watchdog ---

void Watchdog() {
    for (int i = 0; i < 300 && !g_verified && !g_failed; i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    if (!g_verified) {
        std::lock_guard<std::mutex> lock(g_mutex);
        fprintf(stderr, "FAIL [timeout] verified connections: %d/%d\n",
                g_conn_done.load(), kConnCount);
        for (auto& kv : g_progress) {
            fprintf(stderr, "  socket %llu: %llu/%u bytes verified\n",
                    (unsigned long long)kv.first,
                    (unsigned long long)kv.second,
                    (uint32_t)g_expected.size());
        }
        fflush(stderr);
        std::_Exit(1);
    }
}

} // namespace

int main() {
    BuildExpected();

    std::thread watchdog(Watchdog);
    watchdog.detach();

    CppNet server;
    server.Init(2);
    server.SetAcceptCallback(ServerAcceptFunc);
    server.SetReadCallback(ServerReadFunc);
    server.SetDisconnectionCallback(ServerDisconnectFunc);
    if (!server.ListenAndAccept("127.0.0.1", kPort)) {
        Fail("server-listen", "ListenAndAccept failed (port in use?)");
    }

    // let the listen sockets reach the IO threads before connecting
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    CppNet client;
    client.Init(1);
    client.SetConnectionCallback(ClientConnectFunc);
    client.SetReadCallback(ClientReadFunc);
    client.SetDisconnectionCallback(ClientDisconnectFunc);
    for (int i = 0; i < kConnCount; i++) {
        if (!client.Connection("127.0.0.1", kPort)) {
            Fail("client-connect", "Connection() returned false");
        }
    }

    while (!g_verified && !g_failed) {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    printf("PASS: %d connections echoed and verified %u bytes each "
           "(sizes 1K/4K/8K/16K/64K)\n",
           kConnCount, (uint32_t)g_expected.size());
    fflush(stdout);
    std::_Exit(0);
}
