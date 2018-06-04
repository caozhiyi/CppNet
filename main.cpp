#include <iostream>     // std::cout
#include <future>       // std::packaged_task, std::future
#include <chrono>       // std::chrono::seconds
#include <thread>       // std::thread, std::this_thread::sleep_for
#include <thread>
#include <stdarg.h>

#include "Time.h"

class Test {
public:
	int countdown(int from, int to) {
		for (int i = from; i != to; --i) {
			std::cout << i << '\n';
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
		std::cout << "Finished!\n";
		return from - to;
	}

	void Log(const char* pchLog, ...) {
		char chLog[1024] = { 0 };
		va_list ap;
		va_start(ap, pchLog);
		if (vsnprintf(chLog, 1024, pchLog, ap) > 0)
		{
			int a = 0;
			a = 1;
		}
		va_end(ap);
	}

	void Farm() {
		std::packaged_task<int(int, int)> task(std::bind(&Test::countdown, this, 10, 0)); // 设置 packaged_task
		std::future<int> ret = task.get_future(); // 获得与 packaged_task 共享状态相关联的 future 对象.
		//std::thread th(std::move(task), 10, 0);   //创建一个新线程完成计数任务.

		//th.join();
	}
};

//int demo(char *msg, ...)
//{
//	/*定义保存函数参数的结构*/
//	va_list argp;
//	int argno = 0;
//	char *para;
//	/*argp指向传入的第一个可选参数，msg是最后一个确定的参数*/
//	va_start(argp, msg);
//	while (1)
//	{
//		para = va_arg(argp, char *);
//		if (strcmp(para, "\n") == 0)break;
//		printf("Parameter #%d addr=0x%x is: %s\n", argno, para, para);
//		argno++;
//	}
//	va_end(argp);
//	/*将argp置为NULL*/
//	return 0;
//}
int arr[10] = { 9,5,1,22,25,8,99,85,87, 51 };
void PrintArr(int *arr, int len) {
	for (int i = 0; i < len; i++) {
		std::cout << arr[i] << " ";
	}
	std::cout << std::endl;
}

void Swap(int& x, int& y) {
	int temp = x;
	x = y;
	y = temp;
}

void BubbleSort(int *arr, int len) {
	for (int i = 0; i < len; i++) {
		for (int j = 0; j < len - i - 1; j++) {
			if (arr[j] > arr[j+1]) {
				Swap(arr[j], arr[j + 1]);
			}
		}
	}
	PrintArr(arr, len);
}

void SelectionSort(int *arr, int len) {
	for (int i = 0; i < len; i++) {
		int maxIndex = 0;
		for (int j = 0; j < len - i; j++) {
			if (arr[j] > arr[maxIndex]) {
				maxIndex = j;
			}
		}
		Swap(arr[maxIndex], arr[len-i-1]);
	}

	PrintArr(arr, len);
}

void InsertSort(int *arr, int len) {
	for (int i = 0; i < len; i++)
	{
		if (arr[i] < arr[i-1])
		{
			int temp = arr[i];
			int j;
			for (j = i; arr[j - 1] > temp; j--)
			{
				arr[i] = arr[j - 1];
			}
			arr[j] = temp;
		}
	}
	PrintArr(arr, len);
}

void ShellSort(int *arr, int len) {
	for (int step = len / 2; step > 0; step/=2) {
		for (int i = step; i < len; i++) {
			for (int j = i - step; j >= 0 && arr[j] > arr[j + step]; j-=step) {
				Swap(arr[j], arr[j + step]);
			}
		}
	}
}

//将有二个有序数列a[first...mid]和a[mid...last]合并。  
void mergearray(int a[], int first, int mid, int last, int temp[])
{
	int i = first, j = mid + 1;
	int k = 0;

	while (i <= mid && j <= last)
	{
		if (a[i] <= a[j])
			temp[k++] = a[i++];
		else
			temp[k++] = a[j++];
	}

	while (i <= mid)
		temp[k++] = a[i++];

	while (j <= last)
		temp[k++] = a[j++];

	for (i = 0; i < k; i++)
		a[first + i] = temp[i];
}
void mergesort(int a[], int first, int last, int temp[])
{
	if (first < last)
	{
		int mid = (first + last) / 2;
		mergesort(a, first, mid, temp);    //左边有序  
		mergesort(a, mid + 1, last, temp); //右边有序  
		mergearray(a, first, mid, last, temp); //再将二个有序数列合并  
	}
}

bool MergeSort(int a[], int n)
{
	int *p = new int[n];
	if (p == NULL)
		return false;
	mergesort(a, 0, n - 1, p);
	delete[] p;
	PrintArr(a, n);
	return true;
}

void FastSort(int* arr, int start, int end) {
	if (start >= end) {
		return;
	}
	int left = start, right = end;
	int mid = arr[right];
	while (left < right) {
		while (left < right && arr[left] <= mid) {
			left++;
		}
		arr[right] = arr[left];

		while (left < right && arr[right] >= mid) {
			right--;
		}
		arr[left] = arr[right];
	}
	arr[right] = mid;
	FastSort(arr, start, left-1);
	FastSort(arr, left+1, end);
}

//int main() {
//	int arr[6] = { 1,5,9,7,5,3 };
//	FastSort(arr, 0, 5);
//	PrintArr(arr, 6);
//
//	int aaa;
//	std::cin >> aaa;
//}

struct Node {
	int data;
	Node* next;
	Node(int i) :data(i) {}
};

void PrintList(Node* head) {
	Node* temp = head;
	while (temp) {
		std::cout << temp->data << std::endl;
		temp = temp->next;
	}
}

Node* ReversalList(Node* head) {
	if (!head) {
		return NULL;
	}
	Node* left = head;
	Node* mid = head->next;
	if (!mid) {
		return head;
	}
	Node* right = mid->next;
	left->next = NULL;
	while (right) {
		mid->next = left;

		left = mid;
		mid = right;
		right = right->next;
	}
	mid->next = left;
	return mid;
}

#include <string>
//class A {
//public:
//	explicit A(std::string s) { data = s; std::cout << s << std::endl; }
//	A(const A& a) { data = a.data; std::cout << 'B' << std::endl; }
//	A(const A&& a) { data = a.data; std::cout << 'C' << std::endl; }
//	A& operator=(const A& a) { data = a.data; std::cout << 'D' << std::endl; }
//private:
//	std::string data;
//};
//
//A printA(std::string s) {
//	A a(s);
//	return a;
//}

char* m_strcpy(char* des, const char* src) {
	if (!des || !src)
		return NULL;

	char* index = des;
	while (*src != '\0') {
		*des++ = *src++;
	}
	*(des++) = '\0';
	return index;
}

void TestExecpt(int i) {
	if (i > 10) {
		throw std::exception("there have a except");
	}
}

struct A {
	int a;
	virtual ~A() {}
	A() { std::cout << "AAAAAA" << std::endl; }
	virtual void Print() { std::cout << 'B' << std::endl; }
};

struct B: virtual A {
	virtual ~B() {}
	virtual void Print() { std::cout << 'B' << std::endl; }
};

struct C: virtual A {
	virtual ~C() {}
	virtual void Print() { std::cout << 'C' << std::endl; }
};

struct D : C,B {
	int c;
	virtual void Print() { std::cout << 'D' << std::endl; }
};

#include "MemaryPool.h"

class test1 {
public:
	int aaaa;
	int bbbb;
	int cccc;
	int dddd;

	explicit test1(int a, int b, int c, int d):aaaa(a), bbbb(b), cccc(c), dddd(d){
		std::cout << "test1()" << std::endl;
	}
	~test1() {
		std::cout << "~test1()" << std::endl;
	}
};

class test2 {
public:
	int aaaa;

	test2() {
		std::cout << "test2" << std::endl;
	}
	~test2() {
		std::cout << "~test2()" << std::endl;
	}
};

//int main() {
//	CMemaryPool pool(16,2);
//	
//	char* t1 = pool.PoolLargeMalloc<char>();
//
//	int res_t1 = 0;
//	char* t2 = pool.PoolLargeMalloc<char>(20, res_t1);
//
//	int res_t2 = 0;
//	char* t3 = pool.PoolLargeMalloc<char>(40, res_t2);
//
//	pool.PoolLargeFree<char>(t1);
//	pool.PoolLargeFree<char>(t2, res_t1);
//	pool.PoolLargeFree<char>(t3, res_t2);
//	int a;
//	std::cin >> a;
//}

class CString {
public:
	CString():_data(NULL) {

	}

	CString(char* str) {
		if (str) {
			_data = new char[strlen(str) + 1];
			strcpy(_data, str);
		}
	}

	CString(const CString& str) {
		if (str._data) {
			_data = new char[strlen(str._data) + 1];
			strcpy(_data, str._data);
		}
	}

	CString& operator=(const CString& str) {
		if (_data == str._data) {
			return *this;
		}
		if (str._data) {
			if (_data)
				delete[] _data;
			_data = new char[strlen(str._data) + 1];
			strcpy(_data, str._data);
		} else {
			if (_data)
				delete[] _data;
			_data = NULL;
		}
	}

	~CString() {
		if (_data) {
			delete[]_data;
		}
	}

private:
	char* _data;
};

volatile int index = 0;
bool begin = false;
std::mutex	mutex;

void func1() {
	int len = 10000;

	while (!begin) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	while (len--) {
		index++;
	}
}

void func2() {
	int len = 10000;
	while (!begin) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	while (len--) {
		index--;
	}
}

//int main() {
//	std::thread t1 = std::thread(func1);
//	std::thread t2 = std::thread(func2);
//
//	begin = true;
//	t1.join();
//	t2.join();
//
//	std::cout << index << std::endl;
//	int aaa;
//	std::cin >> aaa;
//}
//#include "RunnableShareTaskList.h"
//#include "RunnableAloneTaskListWithPost.h"
//int main() {
//	CRunnableAloneTaskListWithPost run[2];
//	run[0].Start();
//	run[1].Start();
//
//	CMemaryPool* pool = NULL;
//	Task task = [&pool]() {
//		if (pool) {
//			delete pool;
//		}
//	};
//	run[0].Push([&pool]() {
//		pool = new CMemaryPool; 
//		int a = 0;
//		a++;
//	});
//
//	run[1].Push([&pool, task]() { 
//		CRunnableAloneTaskListWithPost::PostTask(pool->GetCreateThreadId(), task);
//	});
//
//	run[0].Join();
//	run[1].Join();
//}
//
//int main() {
//	CRunnableShareTaskList<> run1(1);
//	CRunnableShareTaskList<> run2(1);
//	run1.Start();
//	run2.Start();
//
//
//
//	CMemaryPool* pool = NULL;
//	Task task = [&pool]() {
//		if (pool) {
//			delete pool;
//		}
//	};
//	run1.Push([&pool]() {
//		pool = new CMemaryPool;
//		int a = 0;
//		a++;
//	});
//
//	run1.Push([&pool]() {
//		CRunnableShareTaskList<>::Sleep(1000);
//		if (pool) {
//			delete pool;
//		}
//	});
//
//	run1.Stop();
//	run2.Join();
//}

#include "Time.h"
#include "Log.h"
//int main() {
//	CLog::Instance().Start();
//	CLog::Instance().SetLogName("test");
//	CLog::Instance().SetLogLevel(LOG_DEBUG_LEVEL);
//
//	LOG_DEBUG("%s", "it is a debug log");
//	CRunnable::Sleep(1000);
//	LOG_INFO("%s", "it is a info log");
//	CRunnable::Sleep(1000);
//	LOG_WARN("%s", "it is a warn log");
//	CRunnable::Sleep(1000);
//	LOG_ERROR("%s", "it is a error log");
//	CRunnable::Sleep(1000);
//	LOG_FATAL("%s", "it is a fatal log");
//	CRunnable::Sleep(1000);
//
//	CLog::Instance().Stop();
//	CLog::Instance().Join();
//	int a = 0;
//	a++;
//}

#include "PoolSharedPtr.h"
//int main() {
//	CMemaryPool pool(1024, 10);
//	auto ptr = MakeNewSharedPtr<test1>(pool, 1, 2, 3, 4);
//	ptr->aaaa = 100;
//
//	{
//		auto weak = CMemWeakPtr<test1>(ptr);
//		auto ptrtr = weak.Lock();
//	}
//
//	auto ptr2 = MakeMallocSharedPtr<char>(pool, 55);
//	//strcpy(&(*ptr2), "100000");
//
//	auto ptr3 = MakeLargeSharedPtr<char>(pool);
//	//strcpy(*ptr3, "100000");
//
//	CMemWeakPtr<char> ptr5;
//	{
//		auto ptr4 = MakeMallocSharedPtr<char>(pool, 55);
//		{
//			CMemSharePtr<char> ptr2(ptr4);
//		}
//		{
//			CMemSharePtr<char> ptr2 = ptr4;
//		}
//
//		{
//			ptr5 = ptr4;
//		}
//	}
//
//	auto ptr6 = CMemSharePtr<char>();
//	if (ptr6) {
//		int a = 0;
//		a++;
//	}
//
//	int a = 0;
//	a++;
//}
#include "LoopBuffer.h"
//int main() {
//	std::shared_ptr<CMemaryPool> pool(new CMemaryPool(1024, 20));
//	CLoopBuffer buffer(pool);
//
//	char buf[1024] = { 0 };
//	char buf1[1024] = { 0 };
//	int ttt = 0;
//	for (char* index = buf; index < buf + 1024; index++) {
//		ttt++;
//		if (ttt == 511 || ttt == 512 || ttt == 513) {
//			*index = 'B';
//		} else {
//			*index = 'A';
//		}
//		
//	}
//	int size = sizeof(CLoopBuffer);
//	int len = buffer.Write(buf, 1024);
//	//len = buffer.Write(buf, 100);
//	//len = buffer.Read(buf1, 500);
//	//len = buffer.Write(buf, 700);
//	//len = buffer.Write(buf, 100);
//	//len = buffer.Write(buf, 800);
//	len = buffer.FindStr("BBB", 3);
//	len = buffer.ReadUntil(buf1, 1050);
//	
//	len = buffer.Read(buf1, 1024);
//	len = buffer.Read(buf1, 1024);
//
//
//	//char buf[] = "it is a test in my main function!im";
//	//const char* find = buffer._FindStrInMem(buf, "im", 33, strlen("im"));
//	int a = 0;
//	a++;
//}

#include <fstream>
#include "Buffer.h"
//int main() {
//	std::shared_ptr<CMemaryPool> pool(new CMemaryPool(8, 5));
//	{
//		CBuffer	buffer(pool);
//
//		char buf[] = "This document attempts to describe the general principles and some basic approaches to consider when programming with libcurl. The text will focus mainly on the C interface but might apply fairly well on other interfaces as well as they usually follow the C one pretty closely.\
//	This document will refer to 'the user' as the person writing the source code that uses libcurl.That would probably be you or someone in your position.What will be generally referred to as 'the program' will be the collected source code that you write that is using libcurl for transfers.The program is outside libcurl and libcurl is outside of the program.\
//	To get more details on all options and functions described herein, please refer to their respective man pages.";
//	
//		int times = 10;
//		int write_size = 0;
//		int free_size = buffer.GetFreeSize();
//		while (times--) {
//			write_size += buffer.Write(buf, strlen(buf));
//		}
//		
//		int can_read = buffer.GetCanReadSize();
//		free_size = buffer.GetFreeSize();
//
//		char buf1[8000] = {};
//		int res1 = buffer.Read(buf1, 8000);
//		free_size = buffer.GetFreeSize();
//		int left = 0;
//		int len = 0;
//	
//		times = 10;
//		while (times--) {
//			len += buffer.Write(buf, strlen(buf));
//		}
//		can_read = buffer.GetCanReadSize();
//		int res = buffer.Read(buf1, 8000);
//		buffer.ReleaseUnuseBuffer();
//		times = 10;
//		len = 0;
//		while (times--) {
//			len += buffer.Write(buf, strlen(buf));
//		}
//		int readsizenew = buffer.Read(buf1, 7440);
//	
//		std::ofstream file("file.txt");
//		file << buf1;
//		file.close();
//		int i = 0;
//		i++; 
//	}
//}


#include <map>
#include "Timer.h"

//int main() {
//	std::function<void(void*)> func;
//	func = [](void*) {std::cout << "123456789" << std::endl; };
//	gTimerInstance.AddTimer(10000, func, nullptr);
//	gTimerInstance.AddTimer(20000, func, nullptr);
//	std::vector<Timer> vec;
//	unsigned int tt = 0;
//	while (1){
//		CRunnable::Sleep(tt);
//		tt = gTimerInstance.TimeoutCheck(vec);
//		if (!vec.empty()) {
//			for (size_t i = 0; i < vec.size(); i++) {
//				vec[i]._function(vec[i]._function_param);
//			}
//			vec.clear();
//		}
//		if (!gTimerInstance.GetTimerNum()) {
//			break;
//		}
//	}
//	int a = 0;
//	a++;
//}
