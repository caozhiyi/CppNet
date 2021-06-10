#include "func_thread.h"
#include "info_router.h"

FuncThread::FuncThread(std::shared_ptr<InfoRouter>& router):
    _func_router(router) {
}

FuncThread::~FuncThread() {
}

void FuncThread::Run() {
    while (!_stop) {
        auto t = Pop();
        if (t) {
            if (CallFunc(t->_func_name, t->_func_param_ret)){
                _func_router->PushRet(t);
            }

        } else {
            continue;
        }
    }
}

void FuncThread::Stop() {
    _stop = true;
    Push(nullptr);
}

bool FuncThread::RegisterFunc(const std::string& name, const CommonFunc& func) {
    std::unique_lock<std::mutex> lock(_mutex);
    if (_func_map.count(name)) {
        return false;
    }
    _func_map[name] = func;
    return true;
}

bool FuncThread::RemoveFunc(const std::string& name) {
    std::unique_lock<std::mutex> lock(_mutex);
    auto iter = _func_map.find(name);
    if (iter != _func_map.end()) {
        _func_map.erase(iter);
        return true;
    }
    return false;
}

CommonFunc FuncThread::FindFunc(const std::string& name) {
    std::unique_lock<std::mutex> lock(_mutex);
    auto iter = _func_map.find(name);
    if (iter != _func_map.end()) {
        return iter->second;
    }
    return nullptr;
}

bool FuncThread::CallFunc(const std::string& name, std::vector<cppnet::Any>& param_ret) {
    auto iter = _func_map.find(name);
    if (iter == _func_map.end()) {
        return false;
    }

    //client responsible for parameter verification
    //so can direct call here
    param_ret = iter->second(param_ret);
    return true;
}

