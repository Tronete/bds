#include "thread.h"
int Thread::mgenerated_ID = 0;

Thread::Thread(Threadfunc func) :
	threadfuc(func),
	m_threadID(mgenerated_ID++){
}


int Thread::getID()const {
	return m_threadID;
}

void Thread::start() {
	std::thread t(threadfuc, m_threadID);
	try {
		t.detach();
	}
	catch (const std::exception& ex) {
		std::cout << "Exception caught: " << ex.what() << std::endl;
	}
}