#ifndef COMMON_UTIL_SINGLETON
#define COMMON_UTIL_SINGLETON

namespace cppnet {

template<typename T>
class Singleton {
public:
    static T& Instance() {
        static T instance;
        return instance;
    }

protected:
    Singleton(const Singleton&) {}
    Singleton& operator = (const Singleton&) {}
    Singleton() {}
    virtual ~Singleton() {}
};

}
#endif