#ifndef TEST_HTTP_HTTP_RESPONSE_HEADER
#define TEST_HTTP_HTTP_RESPONSE_HEADER

#include <map>

enum HttpStatusCode {
    kUnknown,
    k200Ok = 200,
    k301MovedPermanently = 301,
    k400BadRequest = 400,
    k404NotFound = 404,
};

namespace base {
    class CBuffer;
}

class CHttpResponse {
    public:
        explicit CHttpResponse(bool close) : _status_code(kUnknown), _close_connection(close) {
        }

        void SetStatusCode(HttpStatusCode code) {
            _status_code = code; 
        }

        void SetStatusMessage(const std::string& message) {
            _status_message = message; 
        }

        void SetCloseConnection(bool on) {
            _close_connection = on; 
        }

        bool GetCloseConnection() const {
            return _close_connection; 
        }

        void SetContentType(const std::string& contentType) {
            AddHeader("Content-Type", contentType);
        }

        // FIXME: replace string with StringPiece
        void AddHeader(const std::string& key, const std::string& value) { 
            _headers_map[key] = value;
        }

        void SetBody(const std::string& body) {
            _body = body;
        }

        std::string GetSendBuffer() const;

    private:
        std::map<std::string, std::string> _headers_map;
        HttpStatusCode _status_code;
        // FIXME: add http version
        std::string _status_message;
        bool _close_connection;
        std::string _body;
};

#endif
