#ifndef HEADER_CPOOLSHAREDPTR
#define HEADER_CPOOLSHAREDPTR
#include <atomic>
#include <functional>
#include <stddef.h>

#include "MemaryPool.h"

class CRefCount;
template<class T>
class CMemWeakPtr;
template<class T>
class CMemSharePtr;


enum MemoryType {
	TYPE_NEW = 0x00,
	TYPE_MALLOC = 0x01,
	TYPE_LARGE = 0x02,
	TYPE_LARGE_WITH_SIZE = 0x03
};

template<class Ty>
class CEnableSharedFromThis {
public:
	typedef Ty _EStype;
	CMemSharePtr<Ty> memshared_from_this() {
		return (_weak_ptr.Lock());
	}

protected:
	constexpr CEnableSharedFromThis() noexcept {
	}

	CEnableSharedFromThis(const CEnableSharedFromThis&) noexcept {
	}

	CEnableSharedFromThis& operator=(const CEnableSharedFromThis&) noexcept {
		return (*this);
	}

	~CEnableSharedFromThis() noexcept {
	}

private:
	template<class T1, class T2>
	friend void DoEnable(T1 *ptr, CEnableSharedFromThis<T2> *es, CRefCount *ref_ptr, CMemoryPool* pool, int size, MemoryType type);
	CMemWeakPtr<Ty> _weak_ptr;
};

// reset internal weak pointer
template<class T1, class T2>
inline void DoEnable(T1 *ptr, CEnableSharedFromThis<T2> *es, CRefCount *ref_ptr, CMemoryPool* pool = nullptr, int size = 0, MemoryType type = TYPE_NEW) {
	es->_weak_ptr.Resetw(ptr, ref_ptr, pool, size, type);
}

//not useful on gcc.
//template<typename T>
//struct has_member_weak_ptr {
//	template <typename _T>
//	static auto check(_T)->typename std::decay<decltype(_T::_weak_ptr)>::type;
//	static void check(...);
//	using type = decltype(check(std::declval<T>()));
//	enum { value = !std::is_void<type>::value };
//};

template<class Ty>
inline void EnableShared(Ty *ptr, CRefCount *ref_ptr, CMemoryPool* pool = nullptr, int size = 0, MemoryType type = TYPE_NEW, typename Ty::_EStype * = 0) {
	if (ptr) {
		DoEnable(ptr, (CEnableSharedFromThis<typename Ty::_EStype>*)ptr, ref_ptr, pool, size, type);
	}
}

inline void EnableShared(const volatile void *, const volatile void *, CMemoryPool* pool = nullptr, int size = 0, MemoryType type = TYPE_NEW) {

}

//reference count class
class CRefCount {
public:
	// construct
	CRefCount() : _uses(1), _weaks(0) {}

	// ensure that derived classes can be destroyed properly
	virtual ~CRefCount() noexcept {}

	// increment use count
	void IncrefUse() {	
		_uses++;
	}

	// increment weak reference count
	void IncrefWeak() {
		_weaks++;
	}

	//decrement use count
	bool DecrefUse() {	
		if (--_uses == 0) {	
			return true;
		}
		return false;
	}

	// decrement weak reference count
	bool DecrefWeak() {
		if (--_weaks == 0) {
			return true;
		}
		return false;
	}

	// return use count
	long GetUseCount() const {	
		return _uses;
	}

	// return weak count
	long GetWeakCount() const {
		return _weaks;
	}

	// return true if _uses == 0
	bool Expired() const  {	
		return (_uses == 0);
	}

private:
	std::atomic_long _uses;
	std::atomic_long _weaks;
};

// base class for CMemSharePtr and CMemWeakPtr
template<typename T>
class CBasePtr {
public:
	typedef CBasePtr<T>	 _BasePtr;

	// construct
	CBasePtr() noexcept : _ptr(nullptr), _ref_count(nullptr), _pool(nullptr) {
		EnableShared(_ptr, _ref_count, _pool);
	}
	CBasePtr(T* ptr, CRefCount* ref, CMemoryPool* pool, MemoryType type, int large_size = 0) noexcept : _ptr(ptr), _ref_count(ref), _pool(pool), _memory_type(type), _malloc_size(large_size) {
		EnableShared(_ptr, _ref_count, _pool, _malloc_size, _memory_type);
	}


	// construct CBasePtr object that takes resource from _Right
	CBasePtr(const _BasePtr& r) : _ptr(r._ptr), _ref_count(r._ref_count), _pool(r._pool), _memory_type(r._memory_type), _malloc_size(r._malloc_size) {
		if (_ref_count) {
			_ref_count->IncrefUse();
		}
		EnableShared(_ptr, _ref_count, _pool, _malloc_size, _memory_type);
	}

	// construct CBasePtr object that takes resource from _Right
	CBasePtr(_BasePtr&& r) : _ptr(r._ptr), _ref_count(r._ref_count), _pool(r._pool), _memory_type(r._memory_type), _malloc_size(r._malloc_size) {
		r._ptr			= nullptr;
		r._ref_count	= nullptr;
		r._pool			= nullptr;
		_malloc_size	= 0;
		EnableShared(_ptr, _ref_count, _pool, _malloc_size, _memory_type);
	}

	// construct CBasePtr object that takes resource from _Right
	_BasePtr& operator=(_BasePtr&& r) {
		if (_ref_count) {
			_ref_count->DecrefUse();
		}
		_ptr = r._ptr;
		_ref_count = r._ref_count;
		_pool = r._pool;
		_memory_type = r._memory_type;
		_malloc_size = r._malloc_size;

		r._ptr = nullptr;
		r._ref_count = nullptr;
		r._pool = nullptr;
		r._malloc_size = 0;
		return (*this);
	}

	// construct CBasePtr object that takes resource from _Right
	_BasePtr& operator=(const _BasePtr& r) {
		if (_ref_count) {
			_ref_count->DecrefUse();
		}

		_ptr		 = r._ptr;
		_ref_count   = r._ref_count;
		_memory_type = r._memory_type;
		_malloc_size = r._malloc_size;
		_pool		 = r._pool;
		if (_ref_count) {
			_ref_count->IncrefUse();
		}
		return (*this);
	}

	// return use count
	long UseCount() const noexcept {	
		return (_ref_count ? _ref_count->GetUseCount() : 0);
	}

	// return pointer to resource
	T* Get() noexcept {
		std::unique_lock<std::mutex> lock(_mutex);
		return (_ptr);
	}

	// test if expired
	bool Expired() noexcept {
		return (!_ref_count || _ref_count->Expired());
	}

	// release resource
	void Reset() {	
		Reset(0, 0);
	}
 
	// release resource and take ownership from CMemWeakPtr _Other._Ptr
	void Reset(const _BasePtr& other) {
		Reset(other._ptr, other._ref_count, other._pool);
	}

	// release resource and take _Other_ptr through _Other_rep
	void Reset(T *other_ptr, CRefCount * other_rep, CMemoryPool* pool) {	
		_Reset0(other_ptr, other_rep, pool);
	}

	// release weak reference to resource
	void Resetw() {
		_Resetw(0, 0);
	}

	// release weak reference to resource and take _Other._Ptr
	void Resetw(_BasePtr& other) {
		Resetw(other._ptr, other._ref_count, other._pool);
	}

	void Resetw(T *other_ptr, CRefCount *other_rep, CMemoryPool* pool) {
		_Resetw0(other_ptr, other_rep, pool);
	}

	void Resetw(T *other_ptr, CRefCount *other_rep, CMemoryPool* pool, int size, MemoryType type) {
		_Resetw0(other_ptr, other_rep, pool, type, size);
	}

public:
	// release resource and take _Other_ptr through _Other_rep
	void Reset(T *other_ptr, CRefCount * other_rep) {
		_Reset0(other_ptr, other_rep);
	}

	// release resource and take new resource
	void _Reset0(T *other_ptr, CRefCount *other_rep) {
		if (other_rep) {
			other_rep->IncrefUse();
		}
		if (_ref_count && _ref_count->GetUseCount() > 0) {
			_DecrefUse();
		}

		std::unique_lock<std::mutex> lock(_mutex);
		_ref_count = other_rep;
		_ptr	   = other_ptr;
		_pool	   = nullptr;
	}

	// release resource and take new resource
	void _Reset0(T *other_ptr, CRefCount *other_rep, CMemoryPool* pool, int size = 0, MemoryType type = TYPE_NEW) {
		if (other_rep) {
			other_rep->IncrefUse();
		}
		if (_ref_count && _ref_count->GetUseCount() > 0) {
			_DecrefUse();
		}

		std::unique_lock<std::mutex> lock(_mutex);
		_ref_count	 = other_rep;
		_ptr		 = other_ptr;
		_pool		 = pool;
		_malloc_size = size;
		_memory_type = type;
	}

	// decrement use reference count
	void _DecrefUse() {
		if (_ref_count && _ref_count->DecrefUse()) {
			_Destroy();
		}
	}

	// decrement use reference count
	void _DecrefWeak() {
		if (_ref_count && _ref_count->DecrefWeak()) {
			_DestroyThis();
		}
	}

	// point to _Other_ptr through _Other_rep
	void _Resetw(T *other_ptr, CRefCount *other_rep) {	
		_Resetw0(other_ptr, other_rep);
	}

	// release resource and take new resource
	void _Resetw0(T *other_ptr, CRefCount *other_rep) {
		if (other_rep) {
			other_rep->IncrefWeak();
		}
		if (_ref_count && _ref_count->GetWeakCount() > 0) {
			_DecrefWeak();
		}

		std::unique_lock<std::mutex> lock(_mutex);
		_ref_count	= other_rep;
		_ptr		= other_ptr;
		_pool		= nullptr;
	}

	// release resource and take new resource
	void _Resetw0(T *other_ptr, CRefCount *other_rep, CMemoryPool* pool) {
		if (other_rep) {
			other_rep->IncrefWeak();
		}
		if (_ref_count && _ref_count->GetWeakCount() > 0) {
			_DecrefWeak();
		}

		std::unique_lock<std::mutex> lock(_mutex);
		_ref_count	= other_rep;
		_ptr		= other_ptr;
		_pool		= pool;
	}

	// release resource and take new resource
	void _Resetw0(T *other_ptr, CRefCount *other_rep, CMemoryPool* pool, MemoryType type, int size) {
		if (other_rep) {
			other_rep->IncrefWeak();
		}
		if (_ref_count && _ref_count->GetWeakCount() > 0) {
			_DecrefWeak();
		}

		std::unique_lock<std::mutex> lock(_mutex);
		_ref_count	 = other_rep;
		_ptr		 = other_ptr;
		_pool		 = pool;
		_memory_type = type;
		_malloc_size = size;
	}

	//release resource
	void _Destroy() noexcept {
		if (_pool) {
			switch (_memory_type) {
			case TYPE_MALLOC:
				_pool->PoolFree<T>(_ptr, _malloc_size);
				break;
			case TYPE_LARGE:
				_pool->PoolLargeFree<T>(_ptr);
				break;
			case TYPE_LARGE_WITH_SIZE:
				_pool->PoolLargeFree<T>(_ptr, _malloc_size);
				break;
			case TYPE_NEW:
				_pool->PoolDelete<T>(_ptr);
				break;
			}

			_pool->PoolDelete<CRefCount>(_ref_count);
		}
	}

	void _DestroyThis() noexcept {
		
	}

	virtual ~CBasePtr() {}

protected:
	T			*_ptr;			//real data ptr
	CRefCount	*_ref_count;
	CMemoryPool	*_pool;			//base memory pool

	int			_malloc_size;	//if malloc large memory from pool. that use to free
	MemoryType	_memory_type;	//malloc memory type from pool

	std::mutex	_mutex;
};

// class for reference counted resource management
template<class T>
class CMemSharePtr : public CBasePtr<T> {	
public:
	typedef CBasePtr<T>	 _BasePtr;
	// construct
	CMemSharePtr() noexcept : _BasePtr() {}
	CMemSharePtr(nullptr_t) noexcept : _BasePtr() {}
	CMemSharePtr(T* ptr, CRefCount* ref, CMemoryPool* pool, MemoryType type, int large_size = 0) noexcept : _BasePtr(ptr, ref, pool, type, large_size) {}

	CMemSharePtr(const _BasePtr& r) : _BasePtr(r) {}
	CMemSharePtr(_BasePtr&& r) : _BasePtr(r) {}

	CMemSharePtr& operator=(_BasePtr&& r) {
		_BasePtr::operator=(r);
		return (*this);
	}

	CMemSharePtr& operator=(const _BasePtr& r) {
		_BasePtr::operator=(r);
		return (*this);
	}

	~CMemSharePtr() {
		this->_DecrefUse();
	}

	_BasePtr& operator==(const _BasePtr& r) noexcept {
		return this->_ptr == r._ptr;
	}

	// return pointer to resource
	T *operator->() noexcept {
		return (this->Get());
	}

	// return pointer to resource
	T& operator*() noexcept {
		return (*(this->Get()));
	}

	// return true if no other CMemSharePtr object owns this resource
	bool unique() const noexcept {
		std::unique_lock<std::mutex> lock(this->_mutex);
		return (this->UseCount() == 1);
	}

	// test if CMemSharePtr object owns no resource
	explicit operator bool() noexcept {
		return (this->Get() != 0);
	}
};

// class for pointer to reference counted resource.
// construc from CMemSharePtr
template<class T>
class CMemWeakPtr : public CBasePtr<T> {
public:
	typedef CBasePtr<T>	 _BasePtr;
	CMemWeakPtr() {
		this->Resetw();
	}

	CMemWeakPtr(_BasePtr& r) {
		this->Resetw(r);
	}

	// construct CBasePtr object that takes resource from _Right
	CMemWeakPtr<T>& operator=(_BasePtr&& r) {
		_BasePtr::operator=(r);
		return (*this);
	}

	// construct CBasePtr object that takes resource from _Right
	CMemWeakPtr<T>& operator=(CMemSharePtr<T>& r) {
		this->Resetw(r);
		return (*this);
	}

	// construct CBasePtr object that takes resource from _Right
	CMemWeakPtr<T>& operator=(CMemWeakPtr<T>& r) {
		this->Resetw(r);
		return (*this);
	}

	// release resource
	~CMemWeakPtr() noexcept {
		this->_DecrefWeak();
	}

	// convert to CMemSharePtr
	CMemSharePtr<T> Lock() noexcept {
		//std::unique_lock<std::mutex> lock(this->_mutex);
		if (this->Expired()) {
			return CMemWeakPtr();
		}
		return (CMemSharePtr<T>(*this));
	}

	// test if CMemSharePtr object owns no resource
	explicit operator bool() noexcept {
		return (this->Get() != 0);
	}
};

//new object on pool
template<typename T, typename... Args >
CMemSharePtr<T> MakeNewSharedPtr(CMemoryPool* pool, Args&&... args) {
	T* o = pool->PoolNew<T>(std::forward<Args>(args)...);
	CRefCount* ref = pool->PoolNew<CRefCount>();
	return CMemSharePtr<T>(o, ref, pool, TYPE_NEW);
}

//malloc from pool
template<typename T>
CMemSharePtr<T> MakeMallocSharedPtr(CMemoryPool* pool, int size) {
	T* o = (T*)pool->PoolMalloc<T>(size);
	CRefCount* ref = pool->PoolNew<CRefCount>();
	return CMemSharePtr<T>(o, ref, pool, TYPE_MALLOC, size);
}

//malloc large memory from pool
template<typename T>
CMemSharePtr<T> MakeLargeSharedPtr(CMemoryPool* pool) {
	T* o = pool->PoolLargeMalloc<T>();
	CRefCount* ref = pool->PoolNew<CRefCount>();
	return CMemSharePtr<T>(o, ref, pool, TYPE_LARGE);
}

template<typename T>
CMemSharePtr<T> MakeLargeSharedPtr(CMemoryPool* pool, int size) {
	T* o = pool->PoolLargeMalloc<T>(size);
	CRefCount* ref = pool->PoolNew<CRefCount>();
	return CMemSharePtr<T>(o, ref, pool, TYPE_LARGE_WITH_SIZE, size);
}
#endif