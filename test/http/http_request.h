#ifndef TEST_HTTP_HTTP_REQUEST_HEADER
#define TEST_HTTP_HTTP_REQUEST_HEADER

#include <ctype.h>
#include <string>
#include <stdio.h>
#include <assert.h>
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
  HttpRequest() : _method(Invalid), _version(Unknown), _path(), _query(), _receive_time(0), _headers_map() { }
  ~HttpRequest() {
    _headers_map.clear();
  }

  void SetVersion(Version v) {
    _version = v;
  }

  Version GetVersion() const { 
    return _version;
  }

  bool SetMethod(const char* start, const char* end) {
    assert(_method == Invalid);
    std::string temp_method(start, end);
    if (temp_method == "GET") {
      _method = Get;

    } else if (temp_method == "POST") {
      _method = Post;

    } else if (temp_method == "HEAD") {
      _method = Head;

    } else if (temp_method == "PUT") {
      _method = Put;

    } else if (temp_method == "DELETE") {
      _method = Delete;

    } else {
      _method = Invalid;
    }
    return _method != Invalid;
  }

  Method GetMethod() const { 
    return _method;
  }

  const char* GetMethodString() const {
    const char* result = "UNKNOWN";
    switch(_method)
    {
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
    _path.assign(start, end);
  }

  const std::string& GetPath() const { 
    return _path;
  }

  void SetQuery(const char* start, const char* end) {
    _query.assign(start, end);
  }

  const std::string& GetQuery() const {
    return _query;
  }

  void SetReceiveTime(uint64_t t) { 
    _receive_time = t;
  }

  uint64_t GetReceiveTime() const { 
    return _receive_time;
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
    _headers_map[field] = value;
  }

  std::string GetHeader(const std::string& field) const {
    std::string result;
    auto it = _headers_map.find(field);
    if (it != _headers_map.end()) {
      result = it->second;
    }
    return result;
  }

  const std::unordered_map<std::string, std::string>& GetHeaders() const { 
    return _headers_map;
  }

  void Clear() {
    _method = Invalid;
    _version = Unknown;
    _headers_map.clear();
    _path.clear();
    _query.clear();
  }

  private:
    Method _method;
    Version _version;
    std::string _path;
    std::string _query;
    uint64_t _receive_time;
    std::unordered_map<std::string, std::string> _headers_map;
};

#endif 