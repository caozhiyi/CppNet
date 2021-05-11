#include "http_server.h"
#include "http_request.h"
#include "http_context.h"
#include "http_response.h"

#include "common/util/time.h"

using namespace cppnet;

HttpServer::HttpServer() {

}

HttpServer::~HttpServer() {

}

void HttpServer::OnConnection(cppnet::Handle handle, uint32_t err) {
    if (err == CEC_SUCCESS) {
        std::unique_lock<std::mutex> lock(_mutex);
        if (_context_map.find(handle) == _context_map.end()) {
            _context_map[handle] = HttpContext();
        }
    }
}

void HttpServer::OnDisConnection(cppnet::Handle handle, uint32_t err) {
    std::unique_lock<std::mutex> lock(_mutex);
    _context_map.erase(handle);
}

void HttpServer::OnMessage(cppnet::Handle handle, cppnet::BufferPtr data, 
                          uint32_t) {
    
    _mutex.lock();
    HttpContext& context = _context_map[handle];
    _mutex.unlock();

    if (!context.ParseRequest(data, cppnet::UTCTimeMsec())) {
        handle->Write("HTTP/1.1 400 Bad Request\r\n\r\n", sizeof("HTTP/1.1 400 Bad Request\r\n\r\n"));
        handle->Close();
    }

    if (context.IsGotAll()) {
        OnRequest(handle, context.GetRequest());
        context.Reset();
    }
}

void HttpServer::OnMessageSend(cppnet::Handle , uint32_t) {
    // do nothing.
}

void HttpServer::OnRequest(cppnet::Handle handle, const HttpRequest& req) {
    const std::string& connection = req.GetHeader("Connection");
    bool close = connection == "close" ||
      (req.GetVersion() == Http10 && connection != "Keep-Alive");

    HttpResponse response(close);
    _http_call_back(req, response);

    std::string res = response.GetSendBuffer();
    handle->Write(res.c_str(), res.length());
    if (response.GetCloseConnection()) {
        handle->Close();
    }
}

