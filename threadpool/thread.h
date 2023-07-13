#ifndef _THREAD_H_
#define _THREAD_H_
#include <iostream>
#include <functional>
#include <thread>


//线程支持的类型
enum class PoolMode {
	MODE_FIXED,
	MODE_CACHED,
};

class Thread {
public:
	//线程函数对象类型
	using Threadfunc = std::function<void(int)>;
	// 线程构造
	Thread(Threadfunc func);
	// 线程析构
	~Thread()=default;
	// 启动线程
	void start();
	//获取线程ID
	int getID()const;

private:
	Threadfunc threadfuc;
	static int mgenerated_ID;
	int m_threadID;
};

#endif