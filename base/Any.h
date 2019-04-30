#ifndef HEADER_CANY
#define HEADER_CANY

#include <algorithm>
#include <typeinfo>

class CAny {
public:
	CAny() noexcept : _content(0) {
	}

	template<typename ValueType>
	CAny(const ValueType & value) : _content(new CHolder<ValueType>(value)) {
	}

	CAny(const CAny & other) : _content(other._content ? other._content->Clone() : 0) {
	}

	CAny(CAny&& other) noexcept : _content(other._content) {
		other._content = 0;
	}

	~CAny() noexcept {
		delete _content;
	}

public: // modifiers
	CAny& Swap(CAny & rhs) noexcept {
		std::swap(_content, rhs._content);
		return *this;
	}

	template<typename ValueType>
	CAny& operator=(const ValueType & rhs) {
		CAny(rhs).Swap(*this);
		return *this;
	}

	CAny & operator=(CAny rhs) {
		CAny(rhs).Swap(*this);
		return *this;
	}

	CAny & operator=(const CAny& rhs) {
		CAny(rhs).Swap(*this);
		return *this;
	}

	// move assignement
	CAny & operator=(CAny&& rhs) noexcept {
		rhs.Swap(*this);
		CAny().Swap(rhs);
		return *this;
	}

public: // queries
	bool Empty() const noexcept {
		return !_content;
	}

	void Clear() noexcept {
		CAny().Swap(*this);
	}

	const std::type_info& Type() const noexcept {
		return _content ? _content->Type() : typeid(void);
	}

	class CPlaceHolder {
	public:
		virtual ~CPlaceHolder() {
		}

		// queries
		virtual const std::type_info& Type() const noexcept = 0;
		virtual CPlaceHolder * Clone() const = 0;
	};

	template<typename ValueType>
	class CHolder : public CPlaceHolder {
	public:
		CHolder(const ValueType& value) : _held(value) {
		}

		CHolder(ValueType&& value) : _held(static_cast<ValueType&&>(value)) {
		}

		// queries
		virtual const std::type_info& Type() const noexcept {
			return typeid(ValueType);
		}
		virtual CPlaceHolder * Clone() const {
			return new CHolder(_held);
		}

	public:
		ValueType _held;

	private:
		CHolder & operator=(const CHolder&);
	};

private: // representation
	template<typename ValueType>
	friend ValueType* any_cast(CAny *) noexcept;

	CPlaceHolder* _content;
};

class bad_any_cast: public std::exception {
public:
	virtual const char * what() const noexcept {
		return "bad_any_cast : failed conversion using any_cast";
	}
};

template<typename ValueType>
ValueType* any_cast(CAny * operand) noexcept {
	if (operand && operand->Type() == typeid(ValueType)) {
		return &static_cast<CAny::CHolder<ValueType> *>(operand->_content)->_held;
	}
	return nullptr;
}

template<typename ValueType>
const ValueType * any_cast(const CAny * operand) noexcept {
	return any_cast<ValueType>(const_cast<CAny *>(operand));
}

template<typename ValueType>
ValueType any_cast(CAny & operand) {
	ValueType * result = any_cast<ValueType>(&operand);
	if (!result)
		std::exception(bad_any_cast());

	return static_cast<ValueType>(*result);
}

template<typename ValueType>
inline ValueType any_cast(const CAny& operand) {
	return any_cast<const ValueType&>(const_cast<CAny&>(operand));
}

template<typename ValueType>
inline ValueType any_cast(CAny&& operand) {
	return any_cast<ValueType>(operand);
}
#endif