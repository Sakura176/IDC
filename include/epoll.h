#ifndef _SERVER_EPOLL_H_
#define _SERVER_EPOLL_H_

#include <sys/epoll.h>
#include "log.h"
#include "macro.h"
#include "threadpool.h"

namespace server
{
	/**
	 * @brief linux epoll IO复用封装类
	 * 
	 */
	class Epoll
	{
	public:
		typedef std::shared_ptr<Epoll> ptr;
		/**
		 * @brief 构造函数，创建epoll句柄
		 *
		 * @param maxEvent
		 */
		Epoll(int maxEvent = 1024);
		~Epoll();
		bool addFd(int fd, uint32_t events);
		bool ModFd(int fd, uint32_t events);
		bool DelFd(int fd);
		int wait(int timeoutMS = -1);
		int getEventFd(size_t i) const;
		uint32_t getEvents(size_t i) const;

	private:
		int m_epollfd;
		std::vector<struct epoll_event> m_events;
	};
}

#endif