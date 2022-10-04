#ifndef __SERVER_THREADPOOL_H__
#define __SERVER_THREADPOOL_H__

#include <queue>
#include <functional>

#include "thread.h"
#include "log.h"

namespace server
{
	class ThreadPool
	{
	public:
		typedef std::shared_ptr<ThreadPool> ptr;

		ThreadPool(size_t threadCount = 5);

		ThreadPool() = default; 
		ThreadPool(ThreadPool&&) = default;
		~ThreadPool();

		void AddTask(std::function<void()> task);

	private:
		static void run(void* arg);

	private:
		struct Pool
		{
			Mutex mtx;
			bool isClosed;
			Semaphore m_semaphore;
			std::queue<std::function<void()>> tasks;
		};
		std::shared_ptr<Pool> pool_;
		Mutex m_mutex;

	};
}



#endif