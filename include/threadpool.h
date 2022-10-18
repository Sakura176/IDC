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

		void addTask(std::function<void()> task);

	private:
		bool m_isClosed;
		Mutex m_mutex;
		Semaphore m_sem;
		std::vector<Thread::ptr> m_workers;
		std::queue<std::function<void()> > m_tasks;
	};
}



#endif