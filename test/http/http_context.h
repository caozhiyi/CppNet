// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)
// Copyright <caozhiyi5@gmail.com>

#ifndef TEST_HTTP_HTTP_CONTEXT_H_
#define TEST_HTTP_HTTP_CONTEXT_H_

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
  HttpContext() : state_(ExpectRequestLine) { }

  // default copy-ctor, dtor and assignment are fine
  // return false if any error
  bool ParseRequest(cppnet::BufferPtr buf, uint64_t receive_time);

  bool IsGotAll() const {
    return state_ == GotAll;
  }

  void Reset() {
    state_ = ExpectRequestLine;
    request_.Clear();
  }

  const HttpRequest& GetRequest() const {
    return request_;
  }

  HttpRequest& GetRequest() {
    return request_;
  }

 private:
  bool processRequestLine(const char* begin, const char* end);

 private:
  HttpRequestParseState state_;
  HttpRequest request_;
};

#endif  // TEST_HTTP_HTTP_CONTEXT_H_

