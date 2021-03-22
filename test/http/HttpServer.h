#ifndef TEST_HTTP_HTTPSERVER_HEADER
#define TEST_HTTP_HTTPSERVER_HEADER

#include <unordered_map>
#include <functional>
#include <mutex>

#include "HttpContext.h"
#include "HttpRequest.h"
#include "HttpResponse.h"

#include "include/cppnet.h"
#include "include/cppnet_socket.h"

typedef std::function<void (const CHttpRequest&, CHttpResponse&)> HttpCallback;

class CHttpServer {
    public:
        CHttpServer();
        ~CHttpServer();

        void SetHttpCallback(const HttpCallback& cb) {
            _http_call_back = cb;
        }

        void OnConnection(cppnet::Handle handle, uint32_t err);

        void OnMessage(cppnet::Handle handle, cppnet::BufferPtr data, 
                          uint32_t len);
      
        void OnMessageSend(cppnet::Handle handle, uint32_t len);
      
    private:
        void OnRequest(cppnet::Handle handle, const CHttpRequest&);

    private:
        std::mutex _mutex;
        std::unordered_map<cppnet::Handle, CHttpContext> _context_map;
        HttpCallback _http_call_back;
};

#endif 
