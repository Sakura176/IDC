#include "../include/server.h"
#include "../include/log.h"
#include <sys/epoll.h>
#include <poll.h>

namespace server
{
	static server::Logger::ptr g_logger = SERVER_LOG_NAME("system");

	TcpServer::TcpServer(
				const std::string host,
				int timeoutMS, 
				std::map<std::string, std::string> sql_params,
				int threadNum,
				int trigMode) :
				m_timeMS(timeoutMS), m_threadpool(new ThreadPool(threadNum)),
				m_sql(new MySQL(sql_params)), m_epoll(new Epoll())
	{
		initEventMode(trigMode);
		if (!init())
		{
			m_isClose = true;
		}

		// todo 设置日志并打印服务器设置信息
	}

	bool TcpServer::init()
	{
		server::IPAddress::ptr addr = server::Address::LookupAnyIPAddress("0.0.0.0:3389");
		if(addr) {
			SERVER_LOG_INFO(g_logger) << "get address: " << addr->toString();
		} else {
			SERVER_LOG_ERROR(g_logger) << "get address fail";
			return false;
		}
		m_sock = Socket::CreateTCP(addr);

		if(!m_sock->bind(addr)) {
			SERVER_LOG_ERROR(g_logger) << "bind fail errno="
				<< errno << " errstr=" << strerror(errno)
				<< " addr=[" << addr->toString() << "]";
			return false;
		}
		if(!m_sock->listen()) {
			SERVER_LOG_ERROR(g_logger) << "listen fail errno="
				<< errno << " errstr=" << strerror(errno)
				<< " addr=[" << addr->toString() << "]";
			return false;
		}

		m_epoll->addFd(m_sock->getSocket(), m_listenEvent | EPOLLIN);

		return true;
	}

	void TcpServer::start()
	{
		int timeMS = -1;
		if (!m_isClose) 
		{
			SERVER_LOG_INFO(g_logger) << "================== Server start ===================";
		}
		while (!m_isClose)
		{
			if(m_timeMS > 0)
			{
				timeMS = m_timer.getNextTimer();
			}
			int eventCnt = m_epoll->wait(timeMS);
			for (int i = 0; i < eventCnt; i++)
			{
				int fd = m_epoll->getEventFd(i);
				if (fd == m_sock->getSocket())
				{
					dealListen();
				}
				else if (m_epoll->getEvents(i) & ( EPOLLRDHUP | EPOLLHUP | EPOLLERR ))
				{
					// TODO 关闭对应的socket连接
				}
				else if (m_epoll->getEvents(i) & EPOLLIN)
				{
					// todo 处理读事件
				}
				else if (m_epoll->getEvents(i) & EPOLLOUT)
				{
					// TODO 处理写事件
				}
			}
		}
	}

	void TcpServer::initEventMode(int trigMode)
	{
		m_listenEvent = EPOLLRDHUP;
		m_connEvent = EPOLLONESHOT | EPOLLRDHUP;
		switch (trigMode)
		{
		case 0:
			break;
		case 1:
			m_connEvent |= EPOLLET;
			break;
		case 2:
			m_listenEvent |= EPOLLET;
			break;
		case 3:
			m_listenEvent |= EPOLLET;
			m_connEvent |= EPOLLET;
			break;
		default:
			m_listenEvent |= EPOLLET;
			m_connEvent |= EPOLLET;
			break;
		}
		HttpConn::isET = (m_connEvent & EPOLLET);
	}

	void TcpServer::dealListen()
	{
		// SERVER_LOG_INFO(g_logger) << "begin deal listen";
		std::cout << "1111111111111111";
		do
		{
			auto client = m_sock->accept();
			if(!client)
			{
				// SERVER_LOG_INFO(g_logger) << "client connect failed";
				return;
			}
			else if (HttpConn::userCount >= 1000)
			{
				// SERVER_LOG_WARN("Client is full!");
				return;
			}
			m_epoll->addFd(client->getSocket(), EPOLLIN);
			clients.emplace_back(client);
			// SERVER_LOG_INFO(g_logger) << "client connect";
		} while (m_listenEvent & EPOLLET);
	}

	void TcpServer::dealTask()
	{
		SERVER_LOG_INFO(g_logger) << "------ dealTask ------ ";
	}


	TcpServer::~TcpServer()
	{
	}
}