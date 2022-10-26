#ifndef __SERVER_TCPSERVER_H__
#define __SERVER_TCPSERVER_H__


#include "log.h"
#include "address.h"
#include "socket.h"
#include "epoll.h"
#include "mysql.h"
#include "macro.h"
#include "timer.h"
#include "httpconn.h"

namespace server
{

	class TcpServer
	{
	public:
		typedef std::shared_ptr<TcpServer> ptr;

		/**
		 * @brief 构造函数，初始化相关参数，准备线程池，数据库连接
		 * 
		 * @param host 
		 * @param timeoutMS 
		 * @param sql_params 
		 * @param threadNum 
		 */
		TcpServer(
				const std::string host,
				int timeoutMS, 
				std::map<std::string, std::string> sql_params,
				int threadNum,
				int trigMode = 3
		);

		void start();

		~TcpServer();
	
	private:
		bool init();
		void initEventMode(int trigMode);

		void dealListen();

		void dealTask();

	private:
		int m_timeMS;
		bool m_isClose;
		int m_sqlPort;

		uint32_t m_listenEvent;
		uint32_t m_connEvent;

		server::ThreadPool::ptr m_threadpool;
		server::MySQL::ptr m_sql;
		server::Epoll::ptr m_epoll;
		server::Socket::ptr m_sock;
		server::TimerManager m_timer;
		std::vector<Socket::ptr> clients;
	};
}

#endif