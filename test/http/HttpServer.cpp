#include "HttpServer.h"
#include "HttpRequest.h"
#include "HttpContext.h"
#include "HttpResponse.h"

#include "common/util/time.h"

using namespace cppnet;

CHttpServer::CHttpServer() {

}

CHttpServer::~CHttpServer() {

}

void CHttpServer::OnConnection(cppnet::Handle handle, uint32_t err) {
    if (err == CEC_SUCCESS) {
        std::unique_lock<std::mutex> lock(_mutex);
        if (_context_map.find(handle) == _context_map.end()) {
            _context_map[handle] = CHttpContext();
        }
    }
}

void CHttpServer::OnMessage(cppnet::Handle handle, cppnet::BufferPtr data, 
                          uint32_t) {
    
    _mutex.lock();
    CHttpContext& context = _context_map[handle];
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

void CHttpServer::OnMessageSend(cppnet::Handle , uint32_t) {
    // do nothing.
}

void CHttpServer::OnRequest(cppnet::Handle handle, const CHttpRequest& req) {
    const std::string& connection = req.GetHeader("Connection");
    bool close = connection == "close" ||
      (req.GetVersion() == Http10 && connection != "Keep-Alive");

    CHttpResponse response(close);
    _http_call_back(req, response);

    std::string res = response.GetSendBuffer();
    handle->Write(res.c_str(), res.length());
    if (response.GetCloseConnection()) {
        handle->Close();
    }
}

