#ifndef __SERVER_THREAD_H__
#define __SERVER_THREAD_H__

#include <pthread.h>
#include <unistd.h>
// #include <thread>
#include <functional>
#include <memory>
#include "util.h"
#include "mutex.h"

namespace server
{
	class Thread : Nocopyable
	{
	public:
		typedef std::shared_ptr<Thread> ptr;

		/**
		 * @brief 构造函数，创建一个线程，同时绑定回调函数和线程名
		 * 
		 * @param cb 回调函数
		 * @param name 线程名
		 */
		Thread(std::function<void()> cb, const std::string& name);

		/**
		 * @brief 析构函数，销毁线程
		 * 
		 */
		~Thread();

		/**
		 * @brief 获取线程ID
		 * 
		 * @return 线程ID
		 */
		long getId() const { return m_id; }

		/**
		 * @brief Get the Name of thread
		 * 
		 * @return const std::string& 
		 */
		const std::string& getName() const { return m_name; }

		/**
		 * @brief 以阻塞的方式等待thread指定的线程结束, 
		 * 
		 */
		void join();

		/**
		 * @brief 分离线程
		 * 
		 */
		void detach();

		/**
		 * @brief 获取线程指针，静态方法
		 * 
		 * @return Thread* 
		 */
		static Thread* GetThis();

		/**
		 * @brief 获取线程名，静态方法
		 * 
		 * @return const std::string& 
		 */
		static const std::string& GetName();

		/**
		 * @brief 设置线程名称，静态方法
		 * 
		 * @param name 
		 */
		static void SetName(const std::string& name);
	private:
		// Thread(const Thread&) = delete;
		// Thread(const Thread&&) = delete;
		// Thread& operator=(const Thread&) = delete;

		/**
		 * @brief 调用线程执行函数
		 * 
		 * @param arg 
		 * @return void* 
		 */
		static void* run(void* arg);
	private:
		// 线程 ID
		long m_id = -1;

		// 线程结构
		pthread_t m_thread = 0;

		// 线程执行函数
		std::function<void()> m_cb;

		// 线程名
		std::string m_name;

		// 信号量
		Semaphore m_semaphore;
	};
}

#endif