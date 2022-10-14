#include "../include/threadpool.h"

namespace server
{
	static server::Logger::ptr g_logger = SERVER_LOG_NAME("system");

	static ThreadPool *t_threadPool = nullptr;
	
	ThreadPool::ThreadPool(size_t threadCount) : pool_(std::make_shared<Pool>())
	{
		for (size_t i = 0; i < threadCount; i++)
		{
			std::function<void()> callback = std::bind(ThreadPool::run, this);
			Thread::ptr thr(new Thread(callback, "name_" + std::to_string(i)));
		}
	}

	ThreadPool::~ThreadPool()
	{
		{
			Mutex::Lock lock(pool_->mtx);
			pool_->isClosed = true;
		}
		// pool_->m_semaphore.notify();
	}

	void ThreadPool::AddTask(std::function<void()> task)			// 右值引用
	{
		{
			// Mutex::Lock lock(pool_->mtx);
			pool_->tasks.emplace(task);
		}
		pool_->m_semaphore.notify();
	}

	void ThreadPool::run(void* arg)
	{
		ThreadPool* threadPool = (ThreadPool*)arg;
		t_threadPool = threadPool;

		std::shared_ptr<Pool> s_pool = t_threadPool->pool_;
		Mutex::Lock lock(s_pool->mtx);

		while(true)
		{
			if(!s_pool->tasks.empty())
			{
				auto task = std::move(s_pool->tasks.front());
				s_pool->tasks.pop();
				task();
			}
			else if(s_pool->isClosed)
				break;
			else
			{
				s_pool->m_semaphore.wait();
			}
		}
	}
}