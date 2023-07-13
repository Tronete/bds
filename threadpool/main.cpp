#include "thread.h"
#include "threadpool.h"

#include <future>
#include <functional>
#include <thread>
#include <chrono>

int sum1(int a, int b)
{
	std::this_thread::sleep_for(std::chrono::seconds(1));
	return a + b;
}

int sum2(int a, int b, int c)
{
	std::this_thread::sleep_for(std::chrono::seconds(1));
	return a + b + c;
}

int main()
{
	ThreadPool pool;
	pool.start(2);

	std::future<int> res1 = pool.submit(sum1, 100, 300);
	std::future<int> res2 = pool.submit(sum2, 1, 2, 3);

	std::cout << "res1:" << res1.get() << std::endl;
	std::cout << "res2:" << res2.get() << std::endl;
}