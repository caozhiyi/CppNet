// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)
// Copyright <caozhiyi5@gmail.com>

#include <string>
#include <cstdio>
#include <utility>
#include "http_response.h"

std::string HttpResponse::GetSendBuffer() const {
  std::string ret;
  ret.append("HTTP/1.1 ");
  ret.append(std::to_string(status_code_));
  ret.append(" ");
  ret.append(status_message_);
  ret.append("\r\n");

  if (close_connection_) {
    ret.append("Connection: close\r\n");

  } else {
    ret.append("Content-Length: ");
    ret.append(std::to_string(body_.size()));
    ret.append("\r\n");
    ret.append("Connection: Keep-Alive\r\n");
  }

  for (const auto& header : headers_map_) {
    ret.append(header.first);
    ret.append(": ");
    ret.append(header.second);
    ret.append("\r\n");
  }
  ret.append("\r\n");
  ret.append(body_);

  return std::move(ret);
}
