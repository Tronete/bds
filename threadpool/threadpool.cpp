#include "threadpool.h"

ThreadPool::ThreadPool():
	init_threadsize(0)
	,idlethreadsize(0)
	,taskSize(0)
	,cur_threadsize(0)
	,taskqueMaxThresHold(TASK_MAX_THRESHHOLD)
	,threadSizeThreshHold(THREAD_MAX_THRESHHOLD)
	,m_poolmode(PoolMode::MODE_FIXED)
,m_isPoolrunning(false)

{}

void ThreadPool::threadFunc(int threadID) {
	auto lasttime = std::chrono::high_resolution_clock().now();
	while (true) {
		Task task;
		{
			//获取锁
			std::unique_lock<std::mutex> lock(m_TaskQueMtx);
			std::cout << "tid:" << std::this_thread::get_id() << "正在尝试获取任务" << std::endl;
			// cached模式下，有可能已经创建了很多的线程，但是空闲时间超过60s，应该把多余的线程
			// 结束回收掉（超过initThreadSize_数量的线程要进行回收）
			// 当前时间 - 上一次线程执行的时间 > 60s
			// 每一秒中返回一次   怎么区分：超时返回？还是有任务待执行返回
			// 锁 + 双重判断

			while (m_taskque.size() == 0) {
				if (!m_isPoolrunning) {
					m_threads.erase(threadID);
					std::cout << "tid:" << std::this_thread::get_id() << "exit" << std::endl;
					m_exitCond.notify_all();
					return;//线程函数结束
				}

				if (m_poolmode == PoolMode::MODE_CACHED) {
					if (std::cv_status::timeout == m_NotEmpty.wait_for(lock, std::chrono::seconds(1))) {
						auto now = std::chrono::high_resolution_clock().now();
						auto dur = std::chrono::duration_cast<std::chrono::seconds>(now - lasttime);
						if (dur.count() >= THREAD_MAX_IDLE_TIME && cur_threadsize > init_threadsize) {
							m_threads.erase(threadID);
							cur_threadsize--;
							idlethreadsize--;
							std::cout << "tid:" << std::thread().get_id() << "exit" << std::endl;
							return;
						}
					}

				}
				else {
					m_NotEmpty.wait(lock);
				}

			}


			idlethreadsize--;
			std::cout << "tid:" << std::this_thread::get_id() << "获取任务成功" << std::endl;

			task = m_taskque.front();
			m_taskque.pop();
			taskSize--;

			if(taskSize>0) m_NotEmpty.notify_all();
			m_NotFull.notify_all();

		}
		if (task != nullptr) {
			task();
		}
		idlethreadsize++;
		lasttime = std::chrono::high_resolution_clock().now();
	}
};

void ThreadPool::start(int initThreadsize) {
	m_isPoolrunning = true;
	init_threadsize = initThreadsize;
	cur_threadsize = initThreadsize;
	for (int i = 0; i < init_threadsize; i++) {
		auto ptr = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc, this, std::placeholders::_1));
		int threadID = ptr->getID();
		std::cout << "threadID:" << threadID << std::endl;
		m_threads.emplace(threadID, std::move(ptr));
	}

	for (int i = 0; i < init_threadsize; i++) {
		m_threads[i]->start();
		idlethreadsize++;
	}
}

bool::ThreadPool::checkRunningState() const 
{
	return m_isPoolrunning;
}