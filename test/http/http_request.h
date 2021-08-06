// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)
// Copyright <caozhiyi5@gmail.com>

#ifndef TEST_HTTP_HTTP_REQUEST_H_
#define TEST_HTTP_HTTP_REQUEST_H_

#include <ctype.h>
#include <string>
#include <cstdio>
#include <utility>
#include <cassert>
#include <unordered_map>

enum Method {
  Invalid,
  Get,
  Post,
  Head,
  Put,
  Delete
};

enum Version {
  Unknown,
  Http10,
  Http11
};

class HttpRequest {
 public:
  HttpRequest(): method_(Invalid), version_(Unknown) {}

  void SetVersion(Version v) {
    version_ = v;
  }

  Version GetVersion() const {
    return version_;
  }

  bool SetMethod(const char* start, const char* end) {
    assert(method_ == Invalid);
    std::string temp_method(start, end);
    if (temp_method == "GET") {
      method_ = Get;

    } else if (temp_method == "POST") {
      method_ = Post;

    } else if (temp_method == "HEAD") {
      method_ = Head;

    } else if (temp_method == "PUT") {
      method_ = Put;

    } else if (temp_method == "DELETE") {
      method_ = Delete;

    } else {
      method_ = Invalid;
    }
    return method_ != Invalid;
  }

  Method GetMethod() const {
    return method_;
  }

  const char* GetMethodString() const {
    const char* result = "UNKNOWN";
    switch (method_) {
      case Get:
        result = "GET";
        break;
      case Post:
        result = "POST";
        break;
      case Head:
        result = "HEAD";
        break;
      case Put:
        result = "PUT";
        break;
      case Delete:
        result = "DELETE";
        break;
      default:
        break;
    }
    return result;
  }

  void SetPath(const char* start, const char* end) {
    path_.assign(start, end);
  }

  const std::string& GetPath() const {
    return path_;
  }

  void SetQuery(const char* start, const char* end) {
    query_.assign(start, end);
  }

  const std::string& GetQuery() const {
    return query_;
  }

  void SetReceiveTime(uint64_t t) {
    receive_time_ = t;
  }

  uint64_t GetReceiveTime() const {
    return receive_time_;
  }

  void AddHeader(const char* start, const char* colon, const char* end) {
    std::string field(start, colon);
    ++colon;
    while (colon < end && isspace(*colon)) {
      ++colon;
    }
    std::string value(colon, end);
    while (!value.empty() && isspace(value[value.size()-1])) {
      value.resize(value.size()-1);
    }
    headers_map_[field] = value;
  }

  std::string GetHeader(const std::string& field) const {
    std::string result;
    auto it = headers_map_.find(field);
    if (it != headers_map_.end()) {
      result = it->second;
    }
    return result;
  }

  const std::unordered_map<std::string, std::string>& GetHeaders() const {
    return headers_map_;
  }

  void Clear() {
    method_ = Invalid;
    version_ = Unknown;
    headers_map_.clear();
    path_.clear();
    query_.clear();
  }

  void Swap(HttpRequest& that) {
    std::swap(method_, that.method_);
    std::swap(version_, that.version_);
    std::swap(receive_time_, that.receive_time_);
    path_.swap(that.path_);
    query_.swap(that.query_);
    headers_map_.swap(that.headers_map_);
  }

 private:
  Method method_;
  Version version_;
  std::string path_;
  std::string query_;
  uint64_t receive_time_;
  std::unordered_map<std::string, std::string> headers_map_;
};

#endif  // TEST_HTTP_HTTP_REQUEST_H_
