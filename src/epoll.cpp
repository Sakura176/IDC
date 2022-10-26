#include "../include/epoll.h"

namespace server
{
	Epoll::Epoll(int maxEvent) : m_epollfd(epoll_create(512)), m_events(maxEvent)
	{
		SERVER_ASSERT(m_epollfd >= 0 && m_events.size() > 0);
	}

	Epoll::~Epoll()
	{
		close(m_epollfd);
	}

	bool Epoll::addFd(int fd, uint32_t events)
	{
		if (fd < 0)
			return false;

		epoll_event ev = {0};
		ev.data.fd = fd;
		ev.events = events;

		return 0 == epoll_ctl(m_epollfd, EPOLL_CTL_ADD, fd, &ev);
	}

	bool Epoll::ModFd(int fd, uint32_t events)
	{
		if (fd < 0)
			return false;

		epoll_event ev = {0};
		ev.data.fd = fd;
		ev.events = events;

		return 0 == epoll_ctl(m_epollfd, EPOLL_CTL_MOD, fd, &ev);
	}

	bool Epoll::DelFd(int fd)
	{
		if (fd < 0)
			return false;

		epoll_event ev = {0};

		return 0 == epoll_ctl(m_epollfd, EPOLL_CTL_DEL, fd, &ev);
	}

	int Epoll::wait(int timeoutMS)
	{
		return epoll_wait(m_epollfd, &m_events[0], m_events.size(), timeoutMS);
	}

	int Epoll::getEventFd(size_t i) const
	{
		SERVER_ASSERT(i < m_events.size() && i >= 0);
		return m_events[i].data.fd;
	}

	uint32_t Epoll::getEvents(size_t i) const
	{
		SERVER_ASSERT(i < m_events.size() && i >= 0);
		return m_events[i].events;
	}
}