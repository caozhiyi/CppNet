#ifndef HEADER_SINGLE
#define HEADER_SINGLE
#include <mutex>
template<typename T>
class CSingle
{
private:
	CSingle(const CSingle&);
	CSingle& operator = (const CSingle&);
public:
	CSingle() {}
	virtual ~CSingle() {}

	static T& Instance() {
		std::unique_lock<std::mutex> lock(_mutex);
		static T instance;
		return instance;
	}

private:
	static std::mutex	_mutex;
};

template<typename T>
std::mutex	CSingle<T>::_mutex;
#endif