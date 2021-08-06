#ifndef TEST_HTTP_HTTPSERVER_HEADER
#define TEST_HTTP_HTTPSERVER_HEADER

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
            _http_call_back = cb;
        }

        void OnConnection(cppnet::Handle handle, uint32_t err);

        void OnDisConnection(cppnet::Handle handle, uint32_t err);

        void OnMessage(cppnet::Handle handle, cppnet::BufferPtr data, 
                          uint32_t len);
      
        void OnMessageSend(cppnet::Handle handle, uint32_t len);
      
    private:
        void OnRequest(cppnet::Handle handle, const HttpRequest&);

    private:
        HttpCallback _http_call_back;
};

#endif 
