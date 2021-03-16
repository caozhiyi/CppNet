#ifndef COMMON_UTIL_ANY
#define COMMON_UTIL_ANY

#include <typeinfo>
#include <algorithm>

namespace cppnet {

class Any {
public:
    Any() noexcept : _content(0) {}
    template<typename ValueType>
    Any(const ValueType & value) : _content(new CHolder<ValueType>(value)) {}
    Any(const Any & other) : _content(other._content ? other._content->Clone() : 0) {}
    Any(Any&& other) noexcept : _content(other._content) {
        other._content = 0;
    }
    ~Any() noexcept {
        delete _content;
    }
public: // modifiers
    Any& Swap(Any & rhs) noexcept {
        std::swap(_content, rhs._content);
        return *this;
    }
    template<typename ValueType>
    Any& operator=(const ValueType & rhs) {
        Any(rhs).Swap(*this);
        return *this;
    }
    Any& operator=(Any rhs) {
        Any(rhs).Swap(*this);
        return *this;
    }
    // move assignement
    Any& operator=(Any&& rhs) noexcept {
        rhs.Swap(*this);
        Any().Swap(rhs);
        return *this;
    }
public: // queries
    bool Empty() const noexcept {
        return !_content;
    }
    void Clear() noexcept {
        Any().Swap(*this);
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
        CHolder & operator=(const CHolder&) {}
    };
private: // representation
    template<typename ValueType>
    friend ValueType* any_cast(Any *) noexcept;
    CPlaceHolder* _content;
};

    class bad_any_cast: public std::exception {
public:
    virtual const char * what() const noexcept {
        return "bad_any_cast : failed conversion using any_cast";
    }
};

template<typename ValueType>
ValueType* any_cast(Any * operand) noexcept {
if (operand && operand->Type() == typeid(ValueType)) {
    return &static_cast<Any::CHolder<ValueType> *>(operand->_content)->_held;
}
return nullptr;


template<typename ValueType>
const ValueType * any_cast(const Any * operand) noexcept {
    return any_cast<ValueType>(const_cast<Any *>(operand));
}

template<typename ValueType>
ValueType any_cast(Any & operand) {
    ValueType * result = any_cast<ValueType>(&operand);
    if (!result)
        std::exception(bad_any_cast());
    return static_cast<ValueType>(*result);
}

template<typename ValueType>
inline ValueType any_cast(const Any& operand) {
    return any_cast<const ValueType&>(const_cast<Any&>(operand));
}

template<typename ValueType>
    inline ValueType any_cast(Any&& operand) {
    return any_cast<ValueType>(operand);
}

}

#endif