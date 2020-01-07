#include "Any.h"
#include "ParsePackage.h"

enum PARSE_STATE {
	PARSE_TYPE	= 1,
	PARSE_NAME	= 2,
	PARSE_NUM	= 3,
	PARSE_PARAM = 4,
	PARSE_CODE	= 5,
	PARSE_FUNC  = 6
};

CParsePackage::CParsePackage() {
}

CParsePackage::~CParsePackage() {
}

bool CParsePackage::ParseType(char* buf, int len, int& type) {
	if (!buf) {
		return false;
	}
	if (*buf < '0' || *buf > '9') {
		return false;
	}

	char typec[4] = { 0 };
	memcpy(typec, buf, 1);
	type = atoi(typec);
	return true;
}

bool CParsePackage::ParseFuncRet(char* buf, int len, int& code, std::string& func_name, const std::map<std::string, std::string>& func_str_map, std::vector<base::CAny>& res) {
	if (!buf) {
		return false;
	}
	char* end = buf + len;
	char* next = buf;
	char* pos = nullptr;
	char* cur = nullptr;

	std::string func_str;
	int state = PARSE_NAME;
	int num = 0;
	int index = 0;
	for (;;) {
		cur = next;
		if (strcmp(cur, "\r\n\r\n") == 0) {
			return true;
		}
		if (cur > end) {
			return false;
		}
		pos = strchr(cur, '|');
		if (pos) {
			next = pos + 1;
			*pos = '\0';

		} else {
			return false;
		}
		switch (state) {
		case PARSE_NAME:
		{
			func_name = cur;
			auto iter = func_str_map.find(func_name);
			if (iter != func_str_map.end()) {
				func_str = iter->second;
				state = PARSE_CODE;
				break;
			}
			return false;
		}

		case PARSE_CODE:
			code = atoi(cur);
			state = PARSE_NUM;
			break;
		case PARSE_NUM:
			num = atoi(cur);
			state = PARSE_PARAM;
			break;
		case PARSE_PARAM:
			if (func_str[index] == 'v') {
				if (_ParseVec(cur, func_str[++index], res)) {
					num--;
					index++;
					if (num == 0 && func_str[index] == ')') 
						return true;
				} else {
					return false;
				}

			} else {
				if (_ParseParam(cur, func_str[index++], res)) {
					num--;
					if (num == 0 && func_str[index] == '(')
						return true;
				} else {
					return false;
				}
			}
			break;
		}
	}
	return true;
}

bool CParsePackage::ParseFuncCall(char* buf, int len, std::string& func_name, const std::map<std::string, std::string>& func_str_map, std::vector<base::CAny>& res) {
	if (!buf) {
		return false;
	}
	char* end = buf + len;
	char* next = buf;
	char* pos = nullptr;
	char* cur = nullptr;

	int state = PARSE_NAME;
	int num = 0;
	std::string func_str;
	size_t index = 0;
	for (;;) {
		cur = next;
		if (strcmp(cur, "\r\n\r\n") == 0) {
			return true;
		}
		if (cur > end) {
			return false;
		}
		pos = strchr(cur, '|');
		if (pos) {
			next = pos + 1;
			*pos = '\0';

		} else {
			return false;
		}
		switch (state) {
		case PARSE_NAME:
		{
			func_name = cur;
			auto iter = func_str_map.find(func_name);
			if (iter != func_str_map.end()) {
				func_str = iter->second;
				state = PARSE_NUM;
				index = func_str.find("(") + 1;
				break;
			}
			return false;
		}
			
		case PARSE_NUM:
			num = atoi(cur);
			state = PARSE_PARAM;
			break;
		case PARSE_PARAM:
			if (func_str[index] == 'v') {
				if (_ParseVec(cur, func_str[++index], res)) {
					num--;
					index++;
					if (num == 0 && func_str[index] == ')') {
						return true;
					}
				} else {
					return false;
				}
			} else {
				if (_ParseParam(cur, func_str[index++], res)) {
					num--;
					if (num == 0 && func_str[index] == ')') {
						return true;
					}
				} else {
					return false;
				}
			}
			break;
		}
	}
	return false;
}

bool CParsePackage::ParseFuncList(char* buf, int len, std::map<std::string, std::string>& map) {
	if (!buf) {
		return false;
	}
	char* end = buf + len;
	char* next = buf;
	char* pos = nullptr;
	char* cur = nullptr;

	int state = PARSE_NUM;
	int num = 0;
	std::string func_name, func_str;

	for (;;) {
		cur = next;
		if (strcmp(cur, "\r\n\r\n") == 0) {
			return true;
		}
		if (cur > end) {
			return false;
		}
		pos = strchr(cur, '|');
		if (pos) {
			next = pos + 1;
			*pos = '\0';
			len = len - (next - cur);

		} else {
			return false;
		}
		switch (state) {
		case PARSE_NUM:
			num = atoi(cur);
			state = PARSE_NAME;
			break;
		case PARSE_NAME:
			func_name = cur;
			state = PARSE_FUNC;
			break;
		case PARSE_FUNC:
			func_str = cur;
			state = PARSE_NAME;
			map[func_name] = func_str;
			num--;
			if (num == 0) {
				return true;
			}
			break;
		}
	}
	return false;
}

bool CParsePackage::PackageFuncRet(char* buf, int& len, int code, const std::string& func_name, const std::map<std::string, std::string>& func_str_map, std::vector<base::CAny>& ret) {
	if (!buf) {
		return false;
	}
	//vector type char two places. so we should remember it
	int v_offset = 0;
	char* end = buf + len;
	char* cur = buf;
	int num = (int)ret.size();
	std::string func_str;
	auto iter = func_str_map.find(func_name);
	if (iter != func_str_map.end()) {
		func_str = iter->second;
	
	} else {
		return false;
	}
	//type
	if (!_SafeSprintf(false, cur, end, "%d|", FUNCTION_RET))
		return false;
	cur += 2;
	//functnion name
	if (!_SafeSprintf(false, cur, end, "%s|", func_name.c_str()))
		return false;
	cur += strlen(cur);
	//error code
	if (!_SafeSprintf(false, cur, end, "%d|", code))
		return false;
	cur += strlen(cur);
	//return value num
	if (!_SafeSprintf(false, cur, end, "%d|", num))
		return false;
	cur += strlen(cur);

	for (int i = 0; i < (int)func_str.size(); i++) {
		if (i - v_offset >= (int)ret.size() && func_str[i] == '(') {
			if (num == 0) {
				if (!_SafeSprintf(false, cur, end, "%s", "\r\n\r\n"))
					return false;
				cur += strlen(cur);
				len = cur - buf;
				return true;
			}
			return false;
		}
		switch (func_str[i]) {
		case 'i':
			if (typeid(int) != ret[i - v_offset].Type()) {
				return false;
			}
            if (!_SafeSprintf(false, cur, end, "i:%d|", base::any_cast<int>(ret[i - v_offset])))
				return false;
			cur += strlen(cur);
			num--;
			break;
		case 'c':
			if (typeid(char) != ret[i - v_offset].Type()) {
				return false;
			}
            if (!_SafeSprintf(false, cur, end, "c:%c|", base::any_cast<char>(ret[i - v_offset])))
				return false;
			cur += strlen(cur);
			num--;
			break;
		case 's':
			if (typeid(std::string) != ret[i - v_offset].Type()) {
				return false;
			}
            if (!_SafeSprintf(false, cur, end, "s:%s|", (base::any_cast<std::string>(ret[i - v_offset])).c_str()))
				return false;
			cur += strlen(cur);
			num--;
			break;
		case 'd':
			if (typeid(double) != ret[i - v_offset].Type()) {
				return false;
			}
            if (!_SafeSprintf(false, cur, end, "d:%f|", base::any_cast<double>(ret[i - v_offset])))
				return false;
			cur += strlen(cur);
			num--;
			break;
		case 'l':
			if (typeid(long) != ret[i - v_offset].Type()) {
				return false;
			}
            if (!_SafeSprintf(false, cur, end, "l:%ld|", base::any_cast<long>(ret[i - v_offset])))
				return false;
			cur += strlen(cur);
			num--;
			break;
		case 'b':
			if (typeid(bool) != ret[i - v_offset].Type()) {
				return false;
			}
            if (!_SafeSprintf(false, cur, end, "b:%d|", base::any_cast<bool>(ret[i - v_offset]) ? 1 : 0))
				return false;
			cur += strlen(cur);
			num--;
			break;
		case 'v':
			i++;
			v_offset++;
			if (!_PackageVec(cur, end, func_str[i], i - v_offset, ret)) {
				return false;
			}
			cur += strlen(cur);
			num--;
			break;
		default:
			return false;
		}
	}
	return false;
}

bool CParsePackage::PackageFuncCall(char* buf, int& len, const std::string& func_name, const std::map<std::string, std::string>& func_str_map, std::vector<base::CAny>& param) {
	if (!buf) {
		return false;
	}
	//vector type char two places. so we should remember it
	int v_offset = 0;
	char* end = buf + len;
	char* cur = buf;
	int num = (int)param.size();
	std::string func_str;
	auto iter = func_str_map.find(func_name);
	if (iter != func_str_map.end()) {
		func_str = iter->second;

	} else {
		return false;
	}
	//type
	if (!_SafeSprintf(false, cur, end, "%d|", FUNCTION_CALL))
		return false;
	cur += 2;
	//functnion name
	if (!_SafeSprintf(false, cur, end, "%s|", func_name.c_str()))
		return false;
	cur += strlen(cur);
	//param num
	if (!_SafeSprintf(false, cur, end, "%d|", num))
		return false;
	cur += strlen(cur);

	int pos = func_str.find("(");
	for (int i = pos + 1; i < (int)func_str.size(); i++) {
		if (i - (pos + 1 + v_offset) >= (int)param.size() && func_str[i] == ')') {
			if (num == 0) {
				if (!_SafeSprintf(false, cur, end, "%s", "\r\n\r\n"))
					return false;
				cur += strlen(cur);
				len = cur - buf;
				return true;
			}
			return false;
		}

		switch (func_str[i]) {
		case 'i':
			if (typeid(int) != param[i - (pos + 1 + v_offset)].Type()) {
				return false;
			}
            if (!_SafeSprintf(false, cur, end, "i:%d|", base::any_cast<int>(param[i - (pos + 1 + v_offset)])))
				return false;
			cur += strlen(cur);
			num--;
			break;
		case 'c':
			if (typeid(char) != param[i - (pos + 1 + v_offset)].Type()) {
				return false;
			}
            if (!_SafeSprintf(false, cur, end, "c:%c|", base::any_cast<char>(param[i - (pos + 1 + v_offset)])))
				return false;
			cur += strlen(cur);
			num--;
			break;
		case 's':
			if (typeid(std::string) != param[i - (pos + 1 + v_offset)].Type()) {
				return false;
			}
            if (!_SafeSprintf(false, cur, end, "s:%s|", (base::any_cast<std::string>(param[i - (pos + 1 + v_offset)])).c_str()))
				return false;
			cur += strlen(cur);
			num--;
			break;
		case 'd':
			if (typeid(double) != param[i - (pos + 1 + v_offset)].Type()) {
				return false;
			}
            if (!_SafeSprintf(false, cur, end, "d:%f|", base::any_cast<double>(param[i - (pos + 1 + v_offset)])))
				return false;
			cur += strlen(cur);
			num--;
			break;
		case 'l':
			if (typeid(long) != param[i - (pos + 1 + v_offset)].Type()) {
				return false;
			}
            if (!_SafeSprintf(false, cur, end, "l:%ld|", base::any_cast<long>(param[i - (pos + 1 + v_offset)])))
				return false;
			cur += strlen(cur);
			num--;
			break;
		case 'b':
			if (typeid(bool) != param[i - (pos + 1 + v_offset)].Type()) {
				return false;
			}
            if (!_SafeSprintf(false, cur, end, "b:%d|", base::any_cast<bool>(param[i - (pos + 1 + v_offset)]) ? 1 : 0))
				return false;
			cur += strlen(cur);
			num--;
			break;
		case 'v':
			i++;
			v_offset++;
			if (!_PackageVec(cur, end, func_str[i], i - (pos + 1 + v_offset), param)) {
				return false;
			}
			cur += strlen(cur);
			num--;
			break;
		default:
			return false;
		}
	}
	return false;
}

bool CParsePackage::PackageFuncList(char* buf, int& len, std::map<std::string, std::string>& func_map) {
	if (!buf) {
		return false;
	}

	char* end = buf + len;
	char* cur = buf;
	int num = (int)func_map.size();
	//type
	if (!_SafeSprintf(false, cur, end, "%d|", FUNCTION_INFO))
		return false;
	cur += 2;
	//function num
	if (!_SafeSprintf(false, cur, end, "%d|", num))
		return false;
	cur += strlen(cur);
	
	for (auto iter = func_map.begin(); iter != func_map.end(); ++iter) {
		if (!_SafeSprintf(false, cur, end, "%s|", iter->first.c_str()))
			return false;
		cur += strlen(cur);

		if (!_SafeSprintf(false, cur, end, "%s|", iter->second.c_str()))
			return false;
		cur += strlen(cur);
	}
	if (!_SafeSprintf(false, cur, end, "%s", "\r\n\r\n"))
		return false;
	cur += strlen(cur);
	len = cur - buf;
	return true;
}

bool CParsePackage::_ParseParam(char* buf, char type, std::vector<base::CAny>& res) {
	char* cur = buf;
	if (*cur != type) {
		return false;
	}
	switch (type)
	{
	case 'i':
        res.push_back(base::CAny(atoi(cur + 2)));
		break;
	case 'c':
        res.push_back(base::CAny(cur + 2));
		break;
	case 's':
        res.push_back(base::CAny(std::string(cur + 2)));
		break;
	case 'd':
        res.push_back(base::CAny(atoll(cur + 2)));
		break;
	case 'l':
        res.push_back(base::CAny(atol(cur + 2)));
		break;
	case 'b':
        res.push_back(base::CAny(strcmp(cur + 2, "1") == 0?true : false));
		break;
	default:
		return false;
	}
	return true;
}

bool CParsePackage::_ParseVec(char* buf, char type, std::vector<base::CAny>& res) {
	char* next = buf;
	char* pos = nullptr;
	char* cur = nullptr;

	//define for different type
	std::vector<int>			i_vec;
	std::vector<char>			c_vec;
	std::vector<std::string>	s_vec;
	std::vector<double>			d_vec;
	std::vector<long>			l_vec;
	std::vector<bool>			b_vec;

	int num = 0;
	PARSE_STATE state = PARSE_NUM;
	for (;;) {
		cur = next;
		pos = strchr(cur, ',');
		if (pos) {
			next = pos + 1;
			*pos = '\0';

		} else {
			if (next)
				cur = next;
			else
				return false;
		}
		if (state == PARSE_NUM) {
			num = atoi(cur);
			state = PARSE_PARAM;

		} else {
			num--;
			switch (type)
			{
			case 'i':
				i_vec.push_back(atoi(cur));
				if (num == 0) {
                    res.push_back(base::CAny(i_vec));
					return true;
				}
				break;
			case 'c':
				c_vec.push_back(*(cur));
				if (num == 0) {
                    res.push_back(base::CAny(c_vec));
					return true;
				}
				break;
			case 's':
				s_vec.push_back(std::string(cur));
				if (num == 0) {
                    res.push_back(base::CAny(s_vec));
					return true;
				}
				break;
			case 'd':
				d_vec.push_back(atoll(cur));
				if (num == 0) {
                    res.push_back(base::CAny(d_vec));
					return true;
				}
				break;
			case 'l':
				l_vec.push_back(atol(cur));
				if (num == 0) {
                    res.push_back(base::CAny(l_vec));
					return true;
				}
				break;
			case 'b':
				b_vec.push_back(strcmp(cur, "1") == 0 ? true : false);
				if (num == 0) {
                    res.push_back(base::CAny(b_vec));
					return true;
				}
				break;
			default:
				return false;
			}
		}
	}
	return false;
}

bool CParsePackage::_PackageVec(char* buf, char* end, char type, int index, std::vector<base::CAny>& vec) {
	char* cur = buf;

	int cur_add = 0;
	int cur_len = 0;

	switch (type) {
	case 'i':
	{
		if (typeid(std::vector<int>) != vec[index].Type()) {
			return false;
		}
        std::vector<int> i_vec = base::any_cast<std::vector<int>>(vec[index]);
		if (!_SafeSprintf(false, cur, end, "%d,", (int)i_vec.size())) {
			return false;
		}
		cur += strlen(cur);

		for (int i = 0; i < (int)i_vec.size(); i++) {
			if (!_SafeSprintf(false, cur, end, ((i == (int)i_vec.size()-1) ? "%d|" : "%d,"), i_vec[i])) {
				return false;
			}
			cur += strlen(cur);
		}
		break;
	}
	case 'c':
	{
		if (typeid(std::vector<char>) != vec[index].Type()) {
			return false;
		}
        std::vector<char> c_vec = base::any_cast<std::vector<char>>(vec[index]);
		if (!_SafeSprintf(false, cur, end, "%d,", (int)c_vec.size())) {
			return false;
		}
		cur += strlen(cur);

		for (int i = 0; i < (int)c_vec.size(); i++) {
			if (!_SafeSprintf(false, cur, end, ((i == (int)c_vec.size() - 1) ? "%c|" : "%c,"), c_vec[i])) {
				return false;
			}
			cur += strlen(cur);
		}
		break;
	}
	case 's':
	{
		if (typeid(std::vector<std::string>) != vec[index].Type()) {
			return false;
		}
        std::vector<std::string> s_vec = base::any_cast<std::vector<std::string>>(vec[index]);
		if (!_SafeSprintf(false, cur, end, "%d,", (int)s_vec.size())) {
			return false;
		}
		cur += strlen(cur);

		for (int i = 0; i < (int)s_vec.size(); i++) {
			if (!_SafeSprintf(true, cur, end, ((i == (int)s_vec.size() - 1) ? "%s|" : "%s,"), s_vec[i].c_str())) {
				return false;
			}
			cur += strlen(cur);
		}
		break;
	}
	case 'd':
	{
		if (typeid(std::vector<double>) != vec[index].Type()) {
			return false;
		}
        std::vector<double> d_vec = base::any_cast<std::vector<double>>(vec[index]);
		if (!_SafeSprintf(false, cur, end, "%d,", (int)d_vec.size())) {
			return false;
		}
		cur += strlen(cur);

		for (int i = 0; i < (int)d_vec.size(); i++) {
			if (!_SafeSprintf(true, cur, end, ((i == (int)d_vec.size() - 1) ? "%lld|" : "%lld,"), d_vec[i])) {
				return false;
			}
			cur += strlen(cur);
		}
		break;
	}
	case 'l':
	{
		if (typeid(std::vector<long>) != vec[index].Type()) {
			return false;
		}
        std::vector<long> l_vec = base::any_cast<std::vector<long>>(vec[index]);
		if (!_SafeSprintf(false, cur, end, "%d,", (int)l_vec.size())) {
			return false;
		}
		cur += strlen(cur);

		for (int i = 0; i < (int)l_vec.size(); i++) {
			if (!_SafeSprintf(true, cur, end, ((i == (int)l_vec.size() - 1) ? "%ld|" : "%ld,"), l_vec[i])) {
				return false;
			}
			cur += strlen(cur);
		}
		break;
	}
	case 'b':
	{
		if (typeid(std::vector<bool>) != vec[index].Type()) {
			return false;
		}
        std::vector<bool> b_vec = base::any_cast<std::vector<bool>>(vec[index]);
		if (!_SafeSprintf(false, cur, end, "%d,", (int)b_vec.size())) {
			return false;
		}
		cur += strlen(cur);

		for (int i = 0; i < (int)b_vec.size(); i++) {
			if (!_SafeSprintf(true, cur, end, ((i == (int)b_vec.size() - 1) ? "%d|" : "%d,"), b_vec[i]?1:0)) {
				return false;
			}
			cur += strlen(cur);
		}
		break;
	}
	default:
		return false;
	}
	return true;;
}
