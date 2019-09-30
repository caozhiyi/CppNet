#ifndef TEST_RPC_CPARSEPACKAGE_HEADER
#define TEST_RPC_CPARSEPACKAGE_HEADER
/********************************************************
message format:
request:
	type|funcntion name|num|param list|\r\n\r\n
  such as:
	1|Add|2|i:50|i:100|\r\n\r\n
	1|Add|1|vi:2,50,100|\r\n\r\n
response:
	type|funcntion name|error code|num|ret list|\r\n\r\n
  such as:
	2|Add|0|1|i:150|\r\n\r\n
	1|Add|0|1|vi:1,100|\r\n\r\n

server send function info to client when client connect
function info:
	type|funcntion num|name|function_str|\r\n\r\n
 such as:
	4|2|Add|i(ii)|Reduce|i(ii)|\r\n\r\n
	
type info:
	i	int
	c	char
	s	string
	d	double
	l	long
	b	bool
	vi	vector<int>
	vc	vector<char>
	...
*********************************************************/
#include <vector>
#include <map>
#include <string.h> // for memset

#include "CommonStruct.h"
#include "Any.h"

class CParsePackage
{
public:
	CParsePackage();
	~CParsePackage();
	//get message type: function call. return. info.
	bool ParseType(char* buf, int len, int& type);
	//parase for every type
    bool ParseFuncRet(char* buf, int len, int& code, std::string& func_name, const std::map<std::string, std::string>& func_str_map, std::vector<base::CAny>& res);
    bool ParseFuncCall(char* buf, int len, std::string& func_name, const std::map<std::string, std::string>& func_str_map, std::vector<base::CAny>& res);
	bool ParseFuncList(char* buf, int len, std::map<std::string, std::string>& map);
	//package for every type
    bool PackageFuncRet(char* buf, int& len, int code, const std::string& func_name, const std::map<std::string, std::string>& func_str_map, std::vector<base::CAny>& ret);
    bool PackageFuncCall(char* buf, int& len, const std::string& func_name, const std::map<std::string, std::string>& func_str_map, std::vector<base::CAny>& param);
	bool PackageFuncList(char* buf, int& len, std::map<std::string, std::string>& func_map);

	template <typename T, typename ...Args>
    void ParseParam(std::vector<base::CAny>& vec, T&& first, Args&&... args);
	template <class T>
    void ParseParam(std::vector<base::CAny>& vec, T&& end);

private:
    bool _ParseParam(char* buf, char type, std::vector<base::CAny>& res);
    bool _ParseVec(char* buf, char type, std::vector<base::CAny>& res);
    bool _PackageVec(char* buf, char* end, char type, int index, std::vector<base::CAny>& vec);

	//format string. max size 4096
	template<typename ...Args>
	bool _SafeSprintf(bool is_str, char* buf, char* end, const char* format, Args&&... args);
};

template<typename ...Args>
bool CParsePackage::_SafeSprintf(bool is_str, char* buf, char* end, const char* format, Args&&... args) {
	if (is_str) {
		char temp_buf[4096] = { 0 };
		sprintf(temp_buf, format, std::forward<Args>(args)...);
		int len = strlen(temp_buf);
		if (len + buf > end) {
			return false;
		}
		sprintf(buf, "%s", temp_buf);
		return true;

	} else {
		char temp_buf[64] = { 0 };
		sprintf(temp_buf, format, std::forward<Args>(args)...);
		int len = strlen(temp_buf);
		if (len + buf > end) {
			return false;
		}
		sprintf(buf, "%s", temp_buf);
		return true;
	}
}

template <typename T, typename ...Args>
void CParsePackage::ParseParam(std::vector<base::CAny>& vec, T&& first, Args&&... args) {
    vec.push_back(base::CAny(std::forward<T>(first)));
	ParseParam(vec, std::forward<Args>(args)...);
}

template <class T>
void CParsePackage::ParseParam(std::vector<base::CAny>& vec, T&& end) {
    vec.push_back(base::CAny(std::forward<T>(end)));
}
#endif
