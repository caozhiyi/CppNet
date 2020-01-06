#ifndef TEST_RPC_CANYFUNC_HEADER
#define TEST_RPC_CANYFUNC_HEADER

#include <vector>
#include <string>
#include <functional>

#include "Any.h"

class CAnyType;
class AnyFunc {
public:
	AnyFunc() : _content(nullptr) {}

	template <class T, class R, typename... Args>
	AnyFunc(R(T::*func)(Args...), T* obj = nullptr) : _content(new MyDelegate<T, R, Args...>(func, obj)) {}

	int Call(std::vector<CAnyType>& param) {
		return _content->Call(param);
	}

	class CFuncBase {
	public:
		CFuncBase() {}
		virtual ~CFuncBase() {}

		virtual int Call(std::vector<CAnyType>& vec) = 0;
	};

	template <class T, class R, typename... Args>
	class  CFunc : public CFuncBase {
	public:
		CFunc(R(T::*func)(Args...), T* obj = nullptr) : _object(obj), _func(func) {}
		~CFunc() {}

		template<typename T>
		T GetValue(T t, CAny& any) {
			return any_cast<ValueType>(any);
		}

		virtual int Call(std::vector<CAnyType>& vec) {
			return (m_t->*m_f)(GetValue(0, vec[0]), GetValue(0, vec[1]));
		}

	private:
		T* _object;
		R(T::*_func)(Args...args);
	};

	CFuncBase* _content;
};

enum ValueType {
	TYPE_INT	= 0x0001,
	TYPE_CHAR	= 0x0002,
	TYPE_STRING = 0x0004,
	TYPE_DOUBLE = 0x0008,
	TYPE_LONG	= 0x0010,
	TYPE_BOOL	= 0x0020,
	TYPE_VECTOR = 0x0040,
};

class CAnyType {
public:
	template<typename T>
	CAnyType(const T& value, ValueType type) : _any(value), _type(type) {}
	~CAnyType() {}

    base::CAny	_any;
	ValueType	_type;
};

#endif
