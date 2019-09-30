#include "FuncThread.h"
#include "InfoRouter.h"

CFuncThread::CFuncThread(std::shared_ptr<CInfoRouter>& router) : _func_router(router) {
}

CFuncThread::~CFuncThread() {
}

void CFuncThread::Run() {
	while (!_stop) {
		auto t = _Pop();
		if (t) {
			if (CallFunc(t->_func_name, t->_func_param_ret)){
				_func_router->PushRet(t);
			}

		} else {
			continue;
		}
	}
}

void CFuncThread::Stop() {
	_stop = true;
	Push(nullptr);
}

bool CFuncThread::RegisterFunc(const std::string& name, const CommonFunc& func) {
    std::unique_lock<std::mutex> lock(_mutex);
	if (_func_map.count(name)) {
		return false;
	}
	_func_map[name] = func;
	return true;
}

bool CFuncThread::RemoveFunc(const std::string& name) {
    std::unique_lock<std::mutex> lock(_mutex);
	auto iter = _func_map.find(name);
	if (iter != _func_map.end()) {
		_func_map.erase(iter);
		return true;
	}
	return false;
}

CommonFunc CFuncThread::FindFunc(const std::string& name) {
    std::unique_lock<std::mutex> lock(_mutex);
	auto iter = _func_map.find(name);
	if (iter != _func_map.end()) {
		return iter->second;
	}
	return nullptr;
}

bool CFuncThread::CallFunc(const std::string& name, std::vector<base::CAny>& param_ret) {
	auto iter = _func_map.find(name);
	if (iter == _func_map.end()) {
		return false;
	}

	//client responsible for parameter verification
	//so can direct call here
	param_ret = iter->second(param_ret);
	return true;
}

