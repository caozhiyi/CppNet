// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)
// Copyright <caozhiyi5@gmail.com>

#ifndef TEST_HTTP_HTTP_SERVER_H_
#define TEST_HTTP_HTTP_SERVER_H_

#include <mutex>
#include <functional>
#include <unordered_map>

#include "http_context.h"
#include "http_request.h"
#include "http_response.h"

#include "include/cppnet.h"

typedef std::function<void (const HttpRequest&, HttpResponse&)> HttpCallback;

class HttpServer {
 public:
  HttpServer();
  ~HttpServer();

  void SetHttpCallback(const HttpCallback& cb) {
    http_call_back_ = cb;
  }

  void OnConnection(cppnet::Handle handle, uint32_t err);

  void OnDisConnection(cppnet::Handle handle, uint32_t err);

  void OnMessage(cppnet::Handle handle, cppnet::BufferPtr data,
            uint32_t len);

  void OnMessageSend(cppnet::Handle handle, uint32_t len);

 private:
  void OnRequest(cppnet::Handle handle, const HttpRequest&);

 private:
  HttpCallback http_call_back_;
};

#endif  // TEST_HTTP_HTTP_SERVER_H_

