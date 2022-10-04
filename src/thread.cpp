#include "../include/thread.h"
#include "../include/log.h"

namespace server
{
	// thread_local 关键字，系统为每一个线程创建该变量的副本
	static thread_local Thread* t_thread = nullptr;
	static thread_local std::string t_thread_name = "UNKNOW";

	static server::Logger::ptr g_logger = SERVER_LOG_NAME("system");

	Thread* Thread::GetThis()
	{
		return t_thread;
	}

	const std::string& Thread::GetName()
	{
		return t_thread_name;
	}

	void Thread::SetName(const std::string& name)
	{
		if(name.empty())
		{
			return;
		}

		if(t_thread)
		{
			t_thread->m_name = name;
		}
		t_thread_name = name;
	}

	Thread::Thread(std::function<void()> cb, const std::string& name)
	: m_cb(cb), m_name(name)
	{
		if(name.empty())
		{
			m_name = "UNKNOW";
		}
		int rt = pthread_create(&m_thread, nullptr, &Thread::run, this);
		if(rt)
		{
			SERVER_LOG_ERROR(g_logger) << "pthread_create thread fail, rt="
				<< rt << " name=" << name;
			throw std::logic_error("pthread_create error");
		}
		m_semaphore.wait();
		// m_cb = 0;
	}

	Thread::~Thread()
	{
		if(m_thread)
		{
			pthread_detach(m_thread);
		}
	}

	void Thread::join()
	{
		if(m_thread)
		{
			int rt = pthread_join(m_thread, nullptr);
			if(rt)
			{
				SERVER_LOG_ERROR(g_logger) << "pthread_join thread fail, rt="
				<< rt << " name=" << m_name;
				throw std::logic_error("pthread_join error");
			}
			m_thread = 0;
		}
	}

	void Thread::detach()
	{
		if(m_thread)
		{
			int rt = pthread_detach(m_thread);
			if(rt)
			{
				SERVER_LOG_ERROR(g_logger) << "pthread_detach thread fail, rt="
				<< rt << " name=" << m_name;
				throw std::logic_error("pthread_detach error");
			}
			m_thread = 0;
		}
	}

	void* Thread::run(void* arg)
	{
		Thread* thread = (Thread*)arg;
		t_thread = thread;
		t_thread_name = thread->m_name;
		thread->m_id = server::GetThreadId();
		pthread_setname_np(pthread_self(), thread->m_name.substr(0, 15).c_str());

		std::function<void()> cb;
		cb.swap(thread->m_cb);

		thread->m_semaphore.notify();

		cb();
		return 0;

	}
}