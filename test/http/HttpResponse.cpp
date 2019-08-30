#include <stdio.h>
#include <string>
#include "HttpResponse.h"

std::string CHttpResponse::GetSendBuffer() const {
    std::string ret;
    ret.append("HTTP/1.1 ");
    ret.append(std::to_string(_status_code));
    ret.append(" ");
    ret.append(_status_message);
    ret.append("\r\n");

    if (_close_connection)
    {
        ret.append("Connection: close\r\n");

    } else {
        ret.append("Content-Length: ");
        ret.append(std::to_string(_body.size()));
        ret.append("\r\n");
        ret.append("Connection: Keep-Alive\r\n");
    }

    for (const auto& header : _headers_map) {
        ret.append(header.first);
        ret.append(": ");
        ret.append(header.second);
        ret.append("\r\n");
    }
    ret.append("\r\n");
    ret.append(_body);

    return std::move(ret);
}
