#ifndef _THREADPOOL_
#define _THREADPOOL_
#include "thread.h"
#include <thread>
#include <queue>
#include <atomic>
#include <memory>
#include <unordered_map>
#include <mutex>
#include <future>

const int TASK_MAX_THRESHHOLD = INT32_MAX;
const int THREAD_MAX_THRESHHOLD = 1024;
const int THREAD_MAX_IDLE_TIME = 60;	//单位：秒

class ThreadPool {
public:
	ThreadPool();
	~ThreadPool() = default;
	void SetThreadMode(PoolMode mode);
	void SetTaskQuMaxHold(int thrhold);
	void SetThreadCachedHold(int Thrhold);
	void start(int initThreadsize = std::thread::hardware_concurrency());

	//template<typename Func, typename... Args>
	template<typename Func, typename... Args>
	auto submit(Func&& func, Args&&... args)->std::future<decltype(func(args...))>
	{
		using Rtype = decltype(func(args...));
		auto task = std::make_shared<std::packaged_task<Rtype()>>(std::bind(std::forward<Func>(func),
		std::forward<Args>(args)...));

		std::future<Rtype> result = task->get_future();

		std::unique_lock<std::mutex> lock(m_TaskQueMtx);

		if (!m_NotFull.wait_for(lock, std::chrono::seconds(1),
			[&]()->bool {return m_taskque.size() < (size_t)taskqueMaxThresHold; }))

		{
			std::cerr << "task queue is full, submit task failed." << std::endl;
			auto task = std::make_shared<std::packaged_task<Rtype()>>([]()->Rtype {return Rtype(); });
			(*task)();
			return task->get_future();
		}
		m_taskque.emplace([task]() {(*task)(); });
		taskSize++;

		m_NotEmpty.notify_all();

		if (m_poolmode == PoolMode::MODE_CACHED && taskSize > idlethreadsize && cur_threadsize < threadSizeThreshHold)
		{
			std::cout << ">>> create new thread..." << std::endl;
			auto ptr = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc, this, std::placeholders::_1));
			int threadID = ptr->getID();
			m_threads.emplace(threadID, std::move(ptr));
			//启动线程
			m_threads[threadID]->start();
			//修改线程个数相关的变量
			cur_threadsize++;
			idlethreadsize++;
		}
		return result;
	}

private:
	void threadFunc(int threadID);
	bool checkRunningState() const;

private:
	int init_threadsize;
	PoolMode m_poolmode;
	std::atomic_int cur_threadsize;
	std::atomic_int idlethreadsize;
	std::atomic_int taskSize;
	std::atomic_bool m_isPoolrunning;
	std::unordered_map <int, std::unique_ptr<Thread>> m_threads;
	using Task = std::function<void()>;
	std::queue<Task> m_taskque;
	std::mutex m_TaskQueMtx;
	std::condition_variable m_NotFull;
	std::condition_variable m_NotEmpty;
	std::condition_variable m_exitCond;
	int taskqueMaxThresHold;
	int threadSizeThreshHold;
};

#endif 

