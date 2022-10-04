/**
 * @file sqlconnpool.h
 * @author your name (you@domain.com)
 * @brief 数据库连接池
 * @version 0.1
 * @date 2022-09-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef _SQLCONNPOOL_H
#define _SQLCONNPOOL_H

#include <queue>
#include <string>
#include <memory>
#include <semaphore.h>
#include <mutex>

#include "_mysql.h"
#include "log.h"
#include "mutex.h"
#include "singleton.h"
#include "util.h"

namespace server
{

class SqlConnPool : public server::Nocopyable
	{
	public:
		typedef std::shared_ptr<SqlConnPool> ptr;

		// static SqlConnPool::ptr Instance();

		Connection::ptr GetConn();

		void FreeConn(Connection::ptr conn);

		int GetFreeConnCount();

		bool Init(const char *connstr,
				const char *charset,
				int connSize = 10);

		void ClosePool();

	// private:
		SqlConnPool();
		~SqlConnPool();
	private:
		int MAX_CONN_;
		int m_useCount;
		int m_freeCount;

		std::queue<std::shared_ptr<Connection> > m_connQue;
		std::mutex m_mtx;
		sem_t m_sem;
	};

	typedef server::Singleton<SqlConnPool> sqlConnPool;
}

#endif