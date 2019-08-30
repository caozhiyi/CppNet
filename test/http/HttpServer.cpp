#include "HttpServer.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "HttpContext.h"

using namespace cppnet;

void DefaultHttpCallback(const CHttpRequest&, CHttpResponse* resp) {
    resp->SetStatusCode(k404NotFound);
    resp->SetStatusMessage("Not Found");
    resp->SetCloseConnection(true);
}

base::CTimeTool CHttpServer::_time_tool;

CHttpServer::CHttpServer() {

}

CHttpServer::~CHttpServer() {

}

void CHttpServer::OnConnection(const cppnet::Handle& handle, uint32_t err) {
    if (err == CEC_SUCCESS) {
        if (_context_map.find(handle) == _context_map.end()) {
            _context_map[handle] = CHttpContext();
        }
    }
}

void CHttpServer::OnMessage(const cppnet::Handle& handle, base::CBuffer* data, 
                          uint32_t len, uint32_t err, bool& continue_read) {

    CHttpContext& context = _context_map[handle];
    _time_tool.Now();
    if (!context.ParseRequest(data, _time_tool.GetMsec())) {
        cppnet::SyncWrite(handle, "HTTP/1.1 400 Bad Request\r\n\r\n", sizeof("HTTP/1.1 400 Bad Request\r\n\r\n"));
        cppnet::Close(handle);
    }

    if (context.gotAll()) {
        OnRequest(handle, context.GetRequest());
        context.Reset();
    }
}

void CHttpServer::OnMessageSend(const cppnet::Handle& handle, uint32_t len, uint32_t err) {
    // do nothing.
}

void CHttpServer::OnRequest(const cppnet::Handle& handle, const CHttpRequest& req) {
    const std::string& connection = req.GetHeader("Connection");
    bool close = connection == "close" ||
      (req.GetVersion() == Http10 && connection != "Keep-Alive");

    CHttpResponse response(close);
    _http_call_back(req, response);

    std::string res = response.GetSendBuffer();
    cppnet::SyncWrite(handle, res.c_str(), res.length());
    if (response.GetCloseConnection()) {
        cppnet::Close(handle);
    }
}

