#ifndef TEST_HTTP_HTTPSERVER_HEADER
#define TEST_HTTP_HTTPSERVER_HEADER

#include <unordered_map>
#include <functional>
#include <mutex>

#include "CppNet.h"
#include "CppDefine.h"
#include "TimeTool.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "HttpContext.h"

typedef std::function<void (const CHttpRequest&, CHttpResponse&)> HttpCallback;

class CHttpServer {
    public:
        CHttpServer();
        ~CHttpServer();

        void SetHttpCallback(const HttpCallback& cb) {
            _http_call_back = cb;
        }

        void OnConnection(const cppnet::Handle& handle, uint32_t err);

        void OnMessage(const cppnet::Handle& handle, base::CBuffer* data, 
                          uint32_t len, uint32_t err);
      
        void OnMessageSend(const cppnet::Handle& handle, uint32_t len, uint32_t err);
      
    private:
        void OnRequest(const cppnet::Handle& handle, const CHttpRequest&);

    private:
        std::mutex _mutex;
        std::unordered_map<cppnet::Handle, CHttpContext> _context_map;
        HttpCallback _http_call_back;

    public:
        static base::CTimeTool _time_tool;
};

#endif 
