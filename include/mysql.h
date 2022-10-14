#ifndef __SERVER_DB_MYSQL_H__
#define __SERVER_DB_MYSQL_H__

#include <mysql.h>
#include <memory>
#include <functional>
#include <map>
#include <list>
#include <vector>
#include "mutex.h"
#include "db.h"
#include "singleton.h"

namespace server
{
	class MySQL;
	class MySQLStmt;

	struct MySQLTime
	{
		MySQLTime(time_t t) : ts(t) {}
		time_t ts;
	};

	/**
	 * @brief MySQL时间转换成系统时间
	 * 
	 * @param mt MySQL时间
	 * @param ts 系统时间
	 * @return true-成功，false-失败
	 */
	bool mysql_time_to_time_t(const MYSQL_TIME &mt, time_t &ts);

	/**
	 * @brief 系统时间转换成MySQL时间
	 * 
	 * @param ts 系统时间
	 * @param mt MySQL时间
	 * @return true-成功，false-失败 
	 */
	bool time_t_to_mysql_time(const time_t &ts, MYSQL_TIME &mt);

	class MySQLRes : public ISQLData
	{
	public:
		typedef std::shared_ptr<MySQLRes> ptr;
		typedef std::function<bool(MYSQL_ROW row, int field_count, int row_no)> data_cb;

		MySQLRes(MYSQL_RES *res, int eno, const char *estr);

		MYSQL_RES *get() const { return m_data.get(); }

		int getErrno() const { return m_errno; }
		const std::string &getErrStr() const { return m_errstr; }

		bool foreach(data_cb cb);

		int getDataCount() override;
		int getColumnCount() override;
		int getColumnBytes(int idx) override;
		int getColumnType(int idx) override;
		std::string getColumnName(int idx) override;

		bool isNull(int idx) override;
		int8_t getInt8(int idx) override;
		uint8_t getUint8(int idx) override;
		int16_t getInt16(int idx) override;
		uint16_t getUint16(int idx) override;
		int32_t getInt32(int idx) override;
		uint32_t getUint32(int idx) override;
		int64_t getInt64(int idx) override;
		uint64_t getUint64(int idx) override;
		float getFloat(int idx) override;
		double getDouble(int idx) override;
		std::string getString(int idx) override;
		std::string getBlob(int idx) override;
		time_t getTime(int idx) override;
		bool next() override;

	private:
		int m_errno;
		std::string m_errstr;
		MYSQL_ROW m_cur;
		unsigned long *m_curLength;
		std::shared_ptr<MYSQL_RES> m_data;
	};

	/**
	 * @brief MYSQL预处理结果类
	 * 
	 */
	class MySQLStmtRes : public ISQLData 
	{
	friend class MySQLStmt;
	public:
		typedef std::shared_ptr<MySQLStmtRes> ptr;
		static MySQLStmtRes::ptr Create(std::shared_ptr<MySQLStmt> stmt);
		~MySQLStmtRes();

		/**
		 * @brief 获取错误代码
		 * 
		 * @return int 
		 */
		int getErrno() const { return m_errno; }

		/**
		 * @brief 获取错误信息
		 * 
		 * @return const std::string& 
		 */
		const std::string& getErrStr() const { return m_errstr; }

		/**
		 * @brief 获取数据条数
		 * 
		 * @return int 
		 */
		int getDataCount() override;

		/**
		 * @brief 获取返回字段的数量
		 * 
		 * @return int 
		 */
		int getColumnCount() override;

		/**
		 * @brief 获取字段数据的长度
		 * 
		 * @param idx 字段索引
		 * @return int 
		 */
		int getColumnBytes(int idx) override;

		/**
		 * @brief 获取字段数据类型
		 * 
		 * @param idx 字段索引
		 * @return int 
		 */
		int getColumnType(int idx) override;

		/**
		 * @brief 获取字段名
		 * 
		 * @param idx 字段索引
		 * @return std::string 
		 */
		std::string getColumnName(int idx) override;

		/**
		 * @brief 判断数据库返回的指定索引下的字段数据是否为空
		 * 
		 * @param idx 字段索引
		 * @return true 
		 */
		bool isNull(int idx) override;
		int8_t getInt8(int idx) override;
		uint8_t getUint8(int idx) override;
		int16_t getInt16(int idx) override;
		uint16_t getUint16(int idx) override;
		int32_t getInt32(int idx) override;
		uint32_t getUint32(int idx) override;
		int64_t getInt64(int idx) override;
		uint64_t getUint64(int idx) override;
		float getFloat(int idx) override;
		double getDouble(int idx) override;
		std::string getString(int idx) override;
		std::string getBlob(int idx) override;
		time_t getTime(int idx) override;
		bool next() override;
	private:
		MySQLStmtRes(std::shared_ptr<MySQLStmt> stmt, int eno, const std::string& estr);
		struct Data 
		{
			Data();
			~Data();

			void alloc(size_t size);

			bool is_null;
			bool error;
			enum_field_types type;
			unsigned long length;
			int32_t data_length;
			char* data;
		};
	private:
		int m_errno;							// 错误代码
		std::string m_errstr;					// 错误信息
		std::shared_ptr<MySQLStmt> m_stmt;		// 预处理类指针
		std::vector<MYSQL_BIND> m_binds;		// 绑定参数数组
		std::vector<Data> m_datas;				// 获取数据数组
	};

	class MySQLManager;

	class MySQL : public IDB, public std::enable_shared_from_this<MySQL> 
	{
	friend class MySQLManager;
	public:
		typedef std::shared_ptr<MySQL> ptr;

		/**
		 * @brief 构造函数，初始化相关参数
		 * 
		 * @param args 
		 */
		MySQL(const std::map<std::string, std::string>& args);

		/**
		 * @brief 初始化数据库
		 * 
		 * @return true 
		 */
		bool connect();

		/**
		 * @brief 简单连接数据库，用于实现长连接
		 * 
		 * @return true 
		 */
		bool ping();

		virtual int execute(const char* format, ...) override;
		/**
		 * @brief 执行查询语句
		 * 
		 * @param format 
		 * @param ap 
		 * @return int 
		 */
		int execute(const char* format, va_list ap);
		virtual int execute(const std::string& sql) override;

		/**
		 * @brief 返回最后一次查询的id
		 * 
		 * @return int64_t 
		 */
		int64_t getLastInsertId() override;

		/**
		 * @brief 获取本类的共享指针
		 * 
		 * @return std::shared_ptr<MySQL> 
		 */
		std::shared_ptr<MySQL> getMySQL();

		/**
		 * @brief 获取MYSQL类的共享指针
		 * 
		 * @return std::shared_ptr<MYSQL> 
		 */
		std::shared_ptr<MYSQL> getRaw();

		/**
		 * @brief 返回受影响或查询的行数
		 * 
		 * @return uint64_t 
		 */
		uint64_t getAffectedRows();

		ISQLData::ptr query(const char* format, ...) override;
		ISQLData::ptr query(const char* format, va_list ap); 
		ISQLData::ptr query(const std::string& sql) override;

		ITransaction::ptr openTransaction(bool auto_commit) override;
		server::IStmt::ptr prepare(const std::string& sql) override;

		template<typename... Args>
		int execStmt(const char* stmt, Args&&... args);

		template<class... Args>
		ISQLData::ptr queryStmt(const char* stmt, Args&&... args);

		/**
		 * @brief 字符串转数组
		 * 
		 * @return const char* 
		 */
		const char* cmd();

		/**
		 * @brief 选择数据库
		 * 
		 * @param dbname 数据库名称
		 * @return true 
		 */
		bool use(const std::string& dbname);

		/**
		 * @brief 获取最近一次数据库调用出错的信息
		 * 
		 * @return int 
		 */
		int getErrno() override;

		/**
		 * @brief 返回包含错误信息的字符串
		 * 
		 * @return std::string 
		 */
		std::string getErrStr() override;

		/**
		 * @brief 获取查询id
		 * 
		 * @return uint64_t 
		 */
		uint64_t getInsertId();
	private:
		/**
		 * @brief 是否需要检查
		 * 
		 * @return true 
		 * @return false 
		 */
		bool isNeedCheck();
	private:
		std::map<std::string, std::string> m_params; 		// 数据库连接参数
		std::shared_ptr<MYSQL> m_mysql;						// MYSQL数据库指针

		std::string m_cmd;
		std::string m_dbname;								// 数据名称

		uint64_t m_lastUsedTime;							// 上次运行时间
		bool m_hasError;									// 是否存在时间
		int32_t m_poolSize;									// 连接池大小
	};

	class MySQLTransaction : public ITransaction 
	{
		public:
			typedef std::shared_ptr<MySQLTransaction> ptr;

			static MySQLTransaction::ptr Create(MySQL::ptr mysql, bool auto_commit);
			~MySQLTransaction();

			bool begin() override;
			bool commit() override;
			bool rollback() override;

			virtual int execute(const char* format, ...) override;
			int execute(const char* format, va_list ap);
			virtual int execute(const std::string& sql) override;
			int64_t getLastInsertId() override;
			std::shared_ptr<MySQL> getMySQL();

			bool isAutoCommit() const { return m_autoCommit;}
			bool isFinished() const { return m_isFinished;}
			bool isError() const { return m_hasError;}
		private:
			MySQLTransaction(MySQL::ptr mysql, bool auto_commit);
		private:
			MySQL::ptr m_mysql;
			bool m_autoCommit;
			bool m_isFinished;
			bool m_hasError;
	};

	/**
	 * @brief MySQL预处理语句类
	 * 
	 */
	class MySQLStmt : public IStmt, public std::enable_shared_from_this<MySQLStmt> 
	{
	public:
		typedef std::shared_ptr<MySQLStmt> ptr;

		/**
		 * @brief 创建MYSQL_STMT句柄，并准备字符串指向的SQL语句
		 * 
		 * @param db 数据库指针
		 * @param stmt SQL语句字符串
		 * @return MySQLStmt::ptr 
		 */
		static MySQLStmt::ptr Create(MySQL::ptr db, const std::string& stmt);

		~MySQLStmt();

		/**
		 * @brief 绑定变量
		 * 
		 * @param idx 索引
		 * @param value 变量
		 * @return int 
		 */
		int bind(int idx, const int8_t& value);
		int bind(int idx, const uint8_t& value);
		int bind(int idx, const int16_t& value);
		int bind(int idx, const uint16_t& value);
		int bind(int idx, const int32_t& value);
		int bind(int idx, const uint32_t& value);
		int bind(int idx, const int64_t& value);
		int bind(int idx, const uint64_t& value);
		int bind(int idx, const float& value);
		int bind(int idx, const double& value);
		int bind(int idx, const std::string& value);
		int bind(int idx, const char* value);
		int bind(int idx, const void* value, int len);
		//int bind(int idx, const MYSQL_TIME& value, int type = MYSQL_TYPE_TIMESTAMP);
		//for null type
		int bind(int idx);

		int bindInt8(int idx, const int8_t& value) override;
		int bindUint8(int idx, const uint8_t& value) override;
		int bindInt16(int idx, const int16_t& value) override;
		int bindUint16(int idx, const uint16_t& value) override;
		int bindInt32(int idx, const int32_t& value) override;
		int bindUint32(int idx, const uint32_t& value) override;
		int bindInt64(int idx, const int64_t& value) override;
		int bindUint64(int idx, const uint64_t& value) override;
		int bindFloat(int idx, const float& value) override;
		int bindDouble(int idx, const double& value) override;
		int bindString(int idx, const char* value) override;
		int bindString(int idx, const std::string& value) override;
		int bindBlob(int idx, const void* value, int64_t size) override;
		int bindBlob(int idx, const std::string& value) override;
		//int bindTime(int idx, const MYSQL_TIME& value, int type = MYSQL_TYPE_TIMESTAMP);
		int bindTime(int idx, const time_t& value) override;
		int bindNull(int idx) override;

		/**
		 * @brief 获取错误代码
		 * 
		 * @return 错误代码 
		 */
		int getErrno() override;

		/**
		 * @brief 获取错误信息
		 * 
		 * @return std::string 错误信息
		 */
		std::string getErrStr() override;

		/**
		 * @brief 执行一次准备好的SQL语句
		 * 
		 * @return 0-成功；其它失败 
		 */
		int execute() override;
		int64_t getLastInsertId() override;
		ISQLData::ptr query() override;

		MYSQL_STMT* getRaw() const { return m_stmt;}
	private:
		MySQLStmt(MySQL::ptr db, MYSQL_STMT* stmt);
	private:
		MySQL::ptr m_mysql;
		MYSQL_STMT* m_stmt;						// MYSQL_STMT 句柄
		std::vector<MYSQL_BIND> m_binds;		// 绑定参数数组
	};

	/**
	 * @brief MySQL数据库管理类
	 * 
	 */
	class MySQLManager 
	{
	public:
		typedef server::Mutex MutexType;

		/**
		 * @brief 初始化
		 */
		MySQLManager();
		~MySQLManager();

		/**
		 * @brief 获取连接池中的mysql数据库连接
		 * 
		 * @param name 
		 * @return MySQL::ptr 
		 */
		MySQL::ptr get(const std::string& name);
		void registerMySQL(const std::string& name, const std::map<std::string, std::string>& params);

		void checkConnection(int sec = 30);

		uint32_t getMaxConn() const { return m_maxConn;}
		void setMaxConn(uint32_t v) { m_maxConn = v;}

		int execute(const std::string& name, const char* format, ...);
		int execute(const std::string& name, const char* format, va_list ap);
		int execute(const std::string& name, const std::string& sql);

		ISQLData::ptr query(const std::string& name, const char* format, ...);
		ISQLData::ptr query(const std::string& name, const char* format, va_list ap); 
		ISQLData::ptr query(const std::string& name, const std::string& sql);

		MySQLTransaction::ptr openTransaction(const std::string& name, bool auto_commit);
	private:
		void freeMySQL(const std::string& name, MySQL* m);
	private:
		uint32_t m_maxConn;
		MutexType m_mutex;
		std::map<std::string, std::list<MySQL*> > m_conns;
		std::map<std::string, std::map<std::string, std::string> > m_dbDefines;
	};

	class MySQLUtil 
	{
		public:
			static ISQLData::ptr Query(const std::string& name, const char* format, ...);
			static ISQLData::ptr Query(const std::string& name, const char* format,va_list ap); 
			static ISQLData::ptr Query(const std::string& name, const std::string& sql);

			static ISQLData::ptr TryQuery(const std::string& name, uint32_t count, const char* format, ...);
			static ISQLData::ptr TryQuery(const std::string& name, uint32_t count, const std::string& sql);

			static int Execute(const std::string& name, const char* format, ...);
			static int Execute(const std::string& name, const char* format, va_list ap); 
			static int Execute(const std::string& name, const std::string& sql);

			static int TryExecute(const std::string& name, uint32_t count, const char* format, ...);
			static int TryExecute(const std::string& name, uint32_t count, const char* format, va_list ap); 
			static int TryExecute(const std::string& name, uint32_t count, const std::string& sql);
	};

	typedef server::Singleton<MySQLManager> MySQLMgr;

	namespace 
	{

		template<size_t N, typename... Args>
		struct MySQLBinder 
		{
			static int Bind(std::shared_ptr<MySQLStmt> stmt) { return 0; }
		};

		template<typename... Args>
		int bindX(MySQLStmt::ptr stmt, Args&... args) 
		{
			return MySQLBinder<1, Args...>::Bind(stmt, args...);
		}
	}

	template<typename... Args>
	int MySQL::execStmt(const char* stmt, Args&&... args) 
	{
		auto st = MySQLStmt::Create(shared_from_this(), stmt);
		if(!st) 
		{
			return -1;
		}
		int rt = bindX(st, args...);
		if(rt != 0) 
		{
			return rt;
		}
		return st->execute();
	}

	template<class... Args>
	ISQLData::ptr MySQL::queryStmt(const char* stmt, Args&&... args) 
	{
		auto st = MySQLStmt::Create(shared_from_this(), stmt);
		if(!st) 
		{
			return nullptr;
		}
		int rt = bindX(st, args...);
		if(rt != 0) 
		{
			return nullptr;
		}
		return st->query();
	}

	namespace 
	{
		template<size_t N, typename Head, typename... Tail>
		struct MySQLBinder<N, Head, Tail...> 
		{
			static int Bind(MySQLStmt::ptr stmt
							,const Head&, Tail&...) 
			{
				//static_assert(false, "invalid type");
				static_assert(sizeof...(Tail) < 0, "invalid type");
				return 0;
			}
		};

		#define XX(type, type2) \
		template<size_t N, typename... Tail> \
		struct MySQLBinder<N, type, Tail...> \
		{ \
			static int Bind(MySQLStmt::ptr stmt \
							, type2 value \
							, Tail&... tail) \
			{ \
				int rt = stmt->bind(N, value); \
				if(rt != 0) { \
					return rt; \
				} \
				return MySQLBinder<N + 1, Tail...>::Bind(stmt, tail...); \
			} \
		};

		XX(char*, char*);
		XX(const char*, char*);
		XX(std::string, std::string&);
		XX(int8_t, int8_t&);
		XX(uint8_t, uint8_t&);
		XX(int16_t, int16_t&);
		XX(uint16_t, uint16_t&);
		XX(int32_t, int32_t&);
		XX(uint32_t, uint32_t&);
		XX(int64_t, int64_t&);
		XX(uint64_t, uint64_t&);
		XX(float, float&);
		XX(double, double&);
		//XX(MYSQL_TIME, MYSQL_TIME&);
		#undef XX
	}
}

#endif