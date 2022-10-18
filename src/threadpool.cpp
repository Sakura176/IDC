#include "../include/threadpool.h"

namespace server
{
	static server::Logger::ptr g_logger = SERVER_LOG_NAME("system");
	
	ThreadPool::ThreadPool(size_t threadCount) : m_isClosed(false)
	{
		for (size_t i = 0; i < threadCount; i++)
		{
			m_workers.emplace_back( 
			new Thread([this]
				{
					while(true)
					{
						std::function<void()> task;
						{
							server::Mutex::Lock lock(this->m_mutex);
							if(!this->m_tasks.empty())
							{
								task = std::move(this->m_tasks.front());
								this->m_tasks.pop();
								task();
							}
							else if(this->m_isClosed && this->m_tasks.empty())
								return;
						}
					}
				}, "thread_" + std::to_string(i))
			);
		};
	}

	ThreadPool::~ThreadPool()
	{
		{
			Mutex::Lock lock(m_mutex);
			m_isClosed = true;
		}
		m_sem.notify();
	}

	void ThreadPool::addTask(std::function<void()> task)			// 右值引用
	{
		Mutex::Lock lock(m_mutex);
		m_tasks.emplace(task);
		m_sem.notify();
	}
}