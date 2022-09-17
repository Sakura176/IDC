#ifndef __SERVER_MUTEX_H__
#define __SERVER_MUTEX_H__

#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <functional>
#include <thread>
#include <memory>
#include <atomic>
#include "../public/util.h"

namespace server
{
	class Semaphore : Nocopyable
	{
	public:
		/**
		 * @brief 构造函数, 初始化信号量
		 * 
		 * @param count 信号量的大小
		 */
		Semaphore(uint32_t count = 0);

		/**
		 * @brief 析构函数，销毁信号量
		 * 
		 */
		~Semaphore();

		/**
		 * @brief 获取信号量
		 * 
		 */
		void wait();

		/**
		 * @brief 释放信号量
		 * 
		 */
		void notify();

	// private:
	// 	Semaphore(const Semaphore&) = delete;
	// 	Semaphore(const Semaphore&&) = delete;
	// 	Semaphore& operator=(const Semaphore&) = delete;
	private:
		sem_t m_semaphore;
	};

	/**
	 * @brief 局部锁的模板实现
	 * 
	 * @tparam T 
	 */
	template<class T>
	struct ScopedLockImpl 
	{
	public:
		/**
		 * @brief 构造函数
		 * 
		 * @param mutex Mutex
		 */
		ScopedLockImpl(T& mutex)
			: m_mutex(mutex)
		{
			m_mutex.lock();
			m_locked = true;
		}

		/**
		 * @brief 析构函数，自动释放锁
		 * 
		 */
		~ScopedLockImpl() { unlock(); }

		/**
		 * @brief 加锁
		 * 
		 */
		void lock()
		{
			if(!m_locked)
			{
				m_mutex.lock();
				m_locked = true;
			}
		}

		/**
		 * @brief 解锁
		 * 
		 */
		void unlock()
		{
			if(m_locked)
			{
				m_mutex.unlock();
				m_locked = false;
			}
		}
	private:
		T& m_mutex;
		bool m_locked; 
	};

	/**
	 * @brief 局部读锁的模板实现
	 * 
	 * @tparam T 
	 */
	template<class T>
	struct ReadScopedLockImpl 
	{
	public:
		/**
		 * @brief 构造函数
		 * 
		 * @param mutex Mutex
		 */
		ReadScopedLockImpl(T& mutex)
			: m_mutex(mutex)
		{
			m_mutex.rdlock();
			m_locked = true;
		}

		/**
		 * @brief 析构函数，自动释放锁
		 * 
		 */
		~ReadScopedLockImpl() { unlock(); }

		/**
		 * @brief 加锁
		 * 
		 */
		void lock()
		{
			if(!m_locked)
			{
				m_mutex.rdlock();
				m_locked = true;
			}
		}

		/**
		 * @brief 解锁
		 * 
		 */
		void unlock()
		{
			if(m_locked)
			{
				m_mutex.unlock();
				m_locked = false;
			}
		}
	private:
		T& m_mutex;
		bool m_locked; 
	};

	/**
	 * @brief 局部写锁的模板实现
	 * 
	 * @tparam T 
	 */
	template<class T>
	struct WriteScopedLockImpl 
	{
	public:
		/**
		 * @brief 构造函数
		 * 
		 * @param mutex Mutex
		 */
		WriteScopedLockImpl(T& mutex)
			: m_mutex(mutex)
		{
			m_mutex.wrlock();
			m_locked = true;
		}

		/**
		 * @brief 析构函数，自动释放锁
		 * 
		 */
		~WriteScopedLockImpl() { unlock(); }

		/**
		 * @brief 加锁
		 * 
		 */
		void lock()
		{
			if(!m_locked)
			{
				m_mutex.wrlock();
				m_locked = true;
			}
		}

		/**
		 * @brief 解锁
		 * 
		 */
		void unlock()
		{
			if(m_locked)
			{
				m_mutex.unlock();
				m_locked = false;
			}
		}
	private:
		T& m_mutex;
		bool m_locked; 
	};

	/**
	 * @brief 互斥量
	 * 
	 */
	class Mutex : Nocopyable
	{
	public:
		// 局部锁
		typedef ScopedLockImpl<Mutex> Lock;

		/**
		 * @brief 构造函数，初始化互斥量
		 * 
		 */
		Mutex()
		{
			pthread_mutex_init(&m_mutex, nullptr);
		}

		/**
		 * @brief 析构函数，销毁互斥量
		 * 
		 */
		~Mutex()
		{
			pthread_mutex_destroy(&m_mutex);
		}

		/**
		 * @brief 加锁
		 * 
		 */
		void lock()
		{
			pthread_mutex_lock(&m_mutex);
		}

		/**
		 * @brief 解锁
		 * 
		 */
		void unlock()
		{
			pthread_mutex_unlock(&m_mutex);
		}
	private:
		// 	互斥量
		pthread_mutex_t m_mutex;
	};

	class RWMutex : Nocopyable
	{
	public:
		typedef ReadScopedLockImpl<RWMutex> ReadLock;

		typedef WriteScopedLockImpl<RWMutex> WriteLock;

		RWMutex()
		{
			pthread_rwlock_init(&m_mutex, nullptr);
		}

		~RWMutex()
		{
			pthread_rwlock_destroy(&m_mutex);
		}

		void rdlock()
		{
			pthread_rwlock_rdlock(&m_mutex);
		}

		void wrlock()
		{
			pthread_rwlock_wrlock(&m_mutex);
		}

		void unlock()
		{
			pthread_rwlock_unlock(&m_mutex);
		}
	private:
		pthread_rwlock_t m_mutex;

	};

}

#endif