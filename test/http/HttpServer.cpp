#include "HttpServer.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "HttpContext.h"

using namespace cppnet;

base::CTimeTool CHttpServer::_time_tool;

CHttpServer::CHttpServer() {

}

CHttpServer::~CHttpServer() {

}

void CHttpServer::OnConnection(const cppnet::Handle& handle, uint32_t err) {
    if (err == CEC_SUCCESS) {
        std::unique_lock<std::mutex> lock(_mutex);
        if (_context_map.find(handle) == _context_map.end()) {
            _context_map[handle] = CHttpContext();
        }
    }
}

void CHttpServer::OnMessage(const cppnet::Handle& handle, base::CBuffer* data, 
                          uint32_t, uint32_t err) {

    if (err != CEC_SUCCESS) {
        return;
    }
    
    _mutex.lock();
    CHttpContext& context = _context_map[handle];
    _mutex.unlock();

    _time_tool.Now();
    if (!context.ParseRequest(data, _time_tool.GetMsec())) {
        cppnet::Write(handle, "HTTP/1.1 400 Bad Request\r\n\r\n", sizeof("HTTP/1.1 400 Bad Request\r\n\r\n"));
        cppnet::Close(handle);
    }

    if (context.IsGotAll()) {
        OnRequest(handle, context.GetRequest());
        context.Reset();
    }
}

void CHttpServer::OnMessageSend(const cppnet::Handle& , uint32_t , uint32_t ) {
    // do nothing.
}

void CHttpServer::OnRequest(const cppnet::Handle& handle, const CHttpRequest& req) {
    const std::string& connection = req.GetHeader("Connection");
    bool close = connection == "close" ||
      (req.GetVersion() == Http10 && connection != "Keep-Alive");

    CHttpResponse response(close);
    _http_call_back(req, response);

    std::string res = response.GetSendBuffer();
    cppnet::Write(handle, res.c_str(), res.length());
    if (response.GetCloseConnection()) {
        cppnet::Close(handle);
    }
}

