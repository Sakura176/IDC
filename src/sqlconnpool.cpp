/**
 * @file sqlconnpool.cpp
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-09-29
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "../include/sqlconnpool.h"

namespace server
{

	static server::Logger::ptr g_logger = SERVER_LOG_NAME("system");

	SqlConnPool::SqlConnPool()
	{
		server::FileLogAppender::ptr file_appender(new server::FileLogAppender("/home/yc/IDC/bin/logfile/sqlconnpool.log"));
		g_logger->addAppender(file_appender);
		m_useCount = 0;
		m_freeCount = 0;
	}

	// SqlConnPool::ptr SqlConnPool::Instance()
	// {
	// 	static SqlConnPool::ptr connPool;
	// 	return connPool;
	// }

	bool SqlConnPool::Init(const char *connstr,
						const char *charset,
						int connSize)
	{
		// assert(connSize > 0);
		for (int i = 0; i < connSize; i++)
		{
			Connection::ptr conn(new Connection());
			if (conn->connecttodb(connstr, charset) != 0)
			{
				SERVER_LOG_ERROR(g_logger) << "MySql init error!";
				return false;
			}
			m_connQue.emplace(conn);
		}
		MAX_CONN_ = connSize;
		sem_init(&m_sem, 0, MAX_CONN_);
		return true;
	}

	Connection::ptr SqlConnPool::GetConn()
	{
		Connection::ptr conn;
		if(m_connQue.empty())
		{
			SERVER_LOG_WARN(g_logger) << "SqlConnPool busy!";
			return nullptr;
		}
		sem_wait(&m_sem);
		{
			std::lock_guard<std::mutex> locker(m_mtx);
			conn = m_connQue.front();
			m_connQue.pop();
		}
		return conn;
	}

	void SqlConnPool::FreeConn(Connection::ptr sql)
	{
		std::lock_guard<std::mutex> locker(m_mtx);
		m_connQue.emplace(sql);
		sem_post(&m_sem);
	}

	void SqlConnPool::ClosePool()
	{
		std::lock_guard<std::mutex> locker(m_mtx);
		while(!m_connQue.empty())
		{
			auto item = m_connQue.front();
			m_connQue.pop();
			item->disconnect();
		}
		mysql_library_end();
	}

	int SqlConnPool::GetFreeConnCount()
	{
		std::lock_guard<std::mutex> locker(m_mtx);
		return m_connQue.size();
	}

	SqlConnPool::~SqlConnPool()
	{
		ClosePool();
	}
}