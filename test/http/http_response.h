// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)
// Copyright <caozhiyi5@gmail.com>

#ifndef TEST_HTTP_HTTP_RESPONSE_H_
#define TEST_HTTP_HTTP_RESPONSE_H_

#include <map>
 #include <string>

enum HttpStatusCode {
  kUnknown,
  k200Ok               = 200,
  k301MovedPermanently = 301,
  k400BadRequest       = 400,
  k404NotFound         = 404,
};

class HttpResponse {
 public:
  explicit HttpResponse(bool close):
    status_code_(kUnknown),
    close_connection_(close) {}

  void SetStatusCode(HttpStatusCode code) {
    status_code_ = code;
  }

  void SetStatusMessage(const std::string& message) {
    status_message_ = message;
  }

  void SetCloseConnection(bool on) {
    close_connection_ = on;
  }

  bool GetCloseConnection() const {
    return close_connection_;
  }

  void SetContentType(const std::string& contentType) {
    AddHeader("Content-Type", contentType);
  }

  // FIXME: replace string with StringPiece
  void AddHeader(const std::string& key, const std::string& value) {
    headers_map_[key] = value;
  }

  void SetBody(const std::string& body) {
    body_ = body;
  }

  std::string GetSendBuffer() const;

 private:
  std::map<std::string, std::string> headers_map_;
  HttpStatusCode status_code_;
  // FIXME: add http version
  std::string status_message_;
  bool close_connection_;
  std::string body_;
};

#endif  // TEST_HTTP_HTTP_RESPONSE_H_
