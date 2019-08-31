#ifndef TEST_HTTP_HTTP_CONTEXT_HEADER
#define TEST_HTTP_HTTP_CONTEXT_HEADER

#include "HttpRequest.h"

enum HttpRequestParseState{
    ExpectRequestLine,
    ExpectHeaders,
    ExpectBody,
    GotAll,
};

namespace base { 
    class CBuffer;
}

class CHttpContext {
    public:
        CHttpContext() : _state(ExpectRequestLine) { }

        // default copy-ctor, dtor and assignment are fine
        // return false if any error
        bool ParseRequest(base::CBuffer* buf, uint64_t receive_time);

        bool IsGotAll() const { 
            return _state == GotAll;
        }

        void Reset() {
            _state = ExpectRequestLine;
            CHttpRequest dummy;
            _request.Swap(dummy);
        }

        const CHttpRequest& GetRequest() const { 
            return _request;
        }

        CHttpRequest& GetRequest(){ 
            return _request;
        }

    private:
        bool processRequestLine(const char* begin, const char* end);
    private:
        HttpRequestParseState _state;
        CHttpRequest _request;
};

#endif
