// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)
// Copyright <caozhiyi5@gmail.com>

#include <algorithm>
#include "http_context.h"
#include "include/cppnet_buffer.h"

const char CRLF[] = "\r\n";
const int CRLF_LEN = 2;
const int VERSION_LEN = sizeof("HTTP/1.1");

bool HttpContext::processRequestLine(const char* begin, const char* end) {
  bool succeed = false;
  const char* start = begin;
  const char* space = std::find(start, end, ' ');
  if (space != end && request_.SetMethod(start, space)) {
    start = space + 1;
    space = std::find(start, end, ' ');
    if (space != end) {
      const char* question = std::find(start, space, '?');
      if (question != space) {
        request_.SetPath(start, question);
        request_.SetQuery(question, space);

      } else {
        request_.SetPath(start, space);
      }

      start = space + 1;
      const char* version_end = start + VERSION_LEN - 1;
      succeed = true;
      if (std::equal(start, version_end, "HTTP/1.1")) {
        request_.SetVersion(Http11);

      } else if (std::equal(start, version_end, "HTTP/1.0")) {
        request_.SetVersion(Http10);

      } else {
        succeed = false;
      }
    }
  }
  return succeed;
}

// return false if any error
bool HttpContext::ParseRequest(cppnet::BufferPtr buf, uint64_t receive_time) {
  bool ok = true;
  bool hasMore = true;
  while (hasMore) {
    if (state_ == ExpectRequestLine) {
      char line_buf[1024] = {0};
      uint32_t need_len = 0;
      uint32_t size = buf->ReadUntil(line_buf, 1024, CRLF, CRLF_LEN, need_len);
      if (size > 0) {
        ok = processRequestLine(line_buf, line_buf + size);
        if (ok) {
          request_.SetReceiveTime(receive_time);
          state_ = ExpectHeaders;

        } else {
          hasMore = false;
        }

      } else {
        hasMore = false;
      }

    } else if (state_ == ExpectHeaders) {
      char line_buf[1024] = { 0 };
      uint32_t need_len = 0;
      uint32_t size = buf->ReadUntil(line_buf, 1024, CRLF, CRLF_LEN, need_len);
      char* end = line_buf + size - CRLF_LEN;
      if (size > 0) {
        const char* colon = std::find(line_buf, end, ':');
        if (colon != end) {
          request_.AddHeader(line_buf, colon, end);

        } else {
          // empty line, end of header
          // FIXME:
          state_ = GotAll;
          hasMore = false;
        }

      } else {
        hasMore = false;
      }

    } else if (state_ == ExpectBody) {
      // TODO:
    }
  }
  return ok;
}
