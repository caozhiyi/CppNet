#ifndef TEST_HTTP_HTTP_CONTEXT_HEADER
#define TEST_HTTP_HTTP_CONTEXT_HEADER

#include "http_request.h"
#include "include/cppnet_type.h"

enum HttpRequestParseState{
    ExpectRequestLine,
    ExpectHeaders,
    ExpectBody,
    GotAll,
};

class HttpContext {
    public:
        HttpContext() : _state(ExpectRequestLine) { }

        // default copy-ctor, dtor and assignment are fine
        // return false if any error
        bool ParseRequest(cppnet::BufferPtr buf, uint64_t receive_time);

        bool IsGotAll() const { 
            return _state == GotAll;
        }

        void Reset() {
            _state = ExpectRequestLine;
            HttpRequest dummy;
            _request.Swap(dummy);
        }

        const HttpRequest& GetRequest() const { 
            return _request;
        }

        HttpRequest& GetRequest(){ 
            return _request;
        }

    private:
        bool processRequestLine(const char* begin, const char* end);
    private:
        HttpRequestParseState _state;
        HttpRequest _request;
};

#endif
