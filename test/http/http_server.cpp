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
        auto context = new HttpContext();
        handle->SetContext((void*)context);
    }
}

void HttpServer::OnDisConnection(cppnet::Handle handle, uint32_t err) {
    auto context = (HttpContext*)handle->GetContext();
    delete context;
}

void HttpServer::OnMessage(cppnet::Handle handle, cppnet::BufferPtr data, 
                          uint32_t) {
    
    auto context = (HttpContext*)handle->GetContext();

    if (!context->ParseRequest(data, cppnet::UTCTimeMsec())) {
        handle->Write("HTTP/1.1 400 Bad Request\r\n\r\n", sizeof("HTTP/1.1 400 Bad Request\r\n\r\n"));
        handle->Close();
    }

    if (context->IsGotAll()) {
        bool is_keep_alive = context->IsKeepAlive();
        OnRequest(handle, !is_keep_alive, context->GetRequest());

        if (is_keep_alive) {
            context->Reset();
        }
    }
}

void HttpServer::OnMessageSend(cppnet::Handle , uint32_t) {
    // do nothing.
}

void HttpServer::OnRequest(cppnet::Handle handle, bool close, const HttpRequest& req) {
    HttpResponse response(close);
    _http_call_back(req, response);

    std::string res = response.GetSendBuffer();
    handle->Write(res.c_str(), (uint32_t)res.length());
    if (response.GetCloseConnection()) {
        handle->Close();
    }
}

