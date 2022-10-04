#pragma once

#include <iostream>
#include <string>
#include <mysql.h>

class MysqlConn
{
public:
	/**
	 * @brief 初始化数据库连接
	 */
	MysqlConn();

	/**
	 * @brief 释放数据库连接
	 */
	~MysqlConn();

	/**
	 * @brief 连接数据库
	 * 
	 * @param user 
	 * @param passwd 
	 * @param dbName 
	 * @param ip 
	 * @param prot 
	 * @return true 
	 * @return false 
	 */
	bool connect(std::string user, std::string passwd, 
			std::string dbName, std::string ip, unsigned short port = 3306);

	/**
	 * @brief 更新数据库
	 * 
	 * @param sql 
	 * @return true 
	 * @return false 
	 */
	bool update(std::string sql);

	/**
	 * @brief 查询数据库
	 * 
	 * @param sql 
	 * @return true 
	 * @return false 
	 */
	bool query(std::string sql);

	/**
	 * @brief 遍历查询得到的结果集
	 * 
	 * @return true 
	 * @return false 
	 */
	bool next();

	/**
	 * @brief 得到结果集中的字段值
	 * 
	 * @param index 
	 * @return std::string 
	 */
	std::string value(int index);

	/**
	 * @brief 事务操作
	 * 
	 * @return true 
	 * @return false 
	 */
	bool transaction();

	/**
	 * @brief 提交事务
	 * 
	 * @return true 
	 * @return false 
	 */
	bool commit();

	/**
	 * @brief 事务回滚
	 * 
	 * @return true 
	 * @return false 
	 */
	bool rollback();
private:
	void freeResult();

	MYSQL *m_conn = nullptr;
	MYSQL_RES *m_result = nullptr;
	MYSQL_ROW m_row = nullptr;
};