#ifndef _THREAD_H_
#define _THREAD_H_
#include <iostream>
#include <functional>
#include <thread>


//�߳�֧�ֵ�����
enum class PoolMode {
	MODE_FIXED,
	MODE_CACHED,
};

class Thread {
public:
	//�̺߳�����������
	using Threadfunc = std::function<void(int)>;
	// �̹߳���
	Thread(Threadfunc func);
	// �߳�����
	~Thread()=default;
	// �����߳�
	void start();
	//��ȡ�߳�ID
	int getID()const;

private:
	Threadfunc threadfuc;
	static int mgenerated_ID;
	int m_threadID;
};

#endif