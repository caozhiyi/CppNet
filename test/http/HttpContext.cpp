#include <algorithm>
#include "Buffer.h"
#include "HttpContext.h"

const char CRLF[] = "\r\n";
const int CRLF_LEN = 2;
const int VERSION_LEN = sizeof("HTTP/1.1");

bool CHttpContext::processRequestLine(const char* begin, const char* end) {
    bool succeed = false;
    const char* start = begin;
    const char* space = std::find(start, end, ' ');
    if (space != end && _request.SetMethod(start, space)) {
        start = space + 1;
        space = std::find(start, end, ' ');
        if (space != end) {
            const char* question = std::find(start, space, '?');
            if (question != space) {
                _request.SetPath(start, question);
                _request.SetQuery(question, space);

            } else {
                _request.SetPath(start, space);
            }

            start = space + 1;
            const char* version_end = start + VERSION_LEN - 1;
            succeed = true;
            if (std::equal(start, version_end, "HTTP/1.1")) {
                _request.SetVersion(Http11);

            } else if (std::equal(start, version_end, "HTTP/1.0")) {
                _request.SetVersion(Http10);

            } else {
                succeed = false;
            }
        }
    }
    return succeed;
}

// return false if any error
bool CHttpContext::ParseRequest(base::CBuffer* buf, uint64_t receive_time) {
    bool ok = true;
    bool hasMore = true;
    while (hasMore) {
        if (_state == ExpectRequestLine) {
            char line_buf[1024] = {0};
            int need_len = 0;
            int size = buf->ReadUntil(line_buf, 1024, CRLF, CRLF_LEN, need_len);
            if (size > 0) {
                ok = processRequestLine(line_buf, line_buf + size);
                if (ok) {
                    _request.SetReceiveTime(receive_time);
                    _state = ExpectHeaders;

                } else {
                    hasMore = false;
                }

            } else {
                hasMore = false;
            }

        } else if (_state == ExpectHeaders) {
            char line_buf[1024] = { 0 };
            int need_len = 0;
            int size = buf->ReadUntil(line_buf, 1024, CRLF, CRLF_LEN, need_len);
            char* end = line_buf + size - CRLF_LEN;
            if (size > 0) {
                const char* colon = std::find(line_buf, end, ':');
                if (colon != end) {
                    _request.AddHeader(line_buf, colon, end);

                } else {
                    // empty line, end of header
                    // FIXME:
                    _state = GotAll;
                    hasMore = false;
                }

            } else {
                hasMore = false;
            }

        } else if (_state == ExpectBody) {
            // TODO:
        }
    }
    return ok;
}
