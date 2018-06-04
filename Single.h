#ifndef HEADER_SINGLE
#define HEADER_SINGLE

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
		static T instance;
		return instance;
	}
};

#endif