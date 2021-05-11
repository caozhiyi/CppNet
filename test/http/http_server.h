#ifndef TEST_HTTP_HTTPSERVER_HEADER
#define TEST_HTTP_HTTPSERVER_HEADER

#include <unordered_map>
#include <functional>
#include <mutex>

#include "http_context.h"
#include "http_request.h"
#include "http_response.h"

#include "include/cppnet.h"
#include "include/cppnet_socket.h"

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
        std::mutex _mutex;
        std::unordered_map<cppnet::Handle, HttpContext> _context_map;
        HttpCallback _http_call_back;
};

#endif 
