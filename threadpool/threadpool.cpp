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
			//��ȡ��
			std::unique_lock<std::mutex> lock(m_TaskQueMtx);
			std::cout << "tid:" << std::this_thread::get_id() << "���ڳ��Ի�ȡ����" << std::endl;
			// cachedģʽ�£��п����Ѿ������˺ܶ���̣߳����ǿ���ʱ�䳬��60s��Ӧ�ðѶ�����߳�
			// �������յ�������initThreadSize_�������߳�Ҫ���л��գ�
			// ��ǰʱ�� - ��һ���߳�ִ�е�ʱ�� > 60s
			// ÿһ���з���һ��   ��ô���֣���ʱ���أ������������ִ�з���
			// �� + ˫���ж�

			while (m_taskque.size() == 0) {
				if (!m_isPoolrunning) {
					m_threads.erase(threadID);
					std::cout << "tid:" << std::this_thread::get_id() << "exit" << std::endl;
					m_exitCond.notify_all();
					return;//�̺߳�������
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
			std::cout << "tid:" << std::this_thread::get_id() << "��ȡ����ɹ�" << std::endl;

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