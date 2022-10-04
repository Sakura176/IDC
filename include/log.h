#ifndef __SERVER_LOG_H_
#define __SERVER_LOG_H_

#include <string>
#include <stdint.h>
#include <memory>
#include <list>
#include <vector>
#include <tuple>
#include <map>
#include <stdarg.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <functional>
#include <map>
#include <iomanip>

#include "singleton.h"
#include "thread.h"


/**
 * @brief 使用流式方式将日志级别level的日志写入到logger
 */
#define SERVER_LOG_LEVEL(logger, level) \
	if(logger->getLevel() <= level) \
		server::LogEventWrap(server::LogEvent::ptr(new server::LogEvent(logger, level, \
						__FILE__, __LINE__, 0, time(0), server::GetThreadId()))).getSS()
				//server::GetFiberId(), time(0), server::Thread::GetName()))).getSS()

/**
 * @brief 使用流式方式将日志级别debug的日志写入到logger
 */
#define SERVER_LOG_DEBUG(logger) SERVER_LOG_LEVEL(logger, server::LogLevel::DEBUG)

/**
 * @brief 使用流式方式将日志级别info的日志写入到logger
 */
#define SERVER_LOG_INFO(logger) SERVER_LOG_LEVEL(logger, server::LogLevel::INFO)

/**
 * @brief 使用流式方式将日志级别warn的日志写入到logger
 */
#define SERVER_LOG_WARN(logger) SERVER_LOG_LEVEL(logger, server::LogLevel::WARN)

/**
 * @brief 使用流式方式将日志级别error的日志写入到logger
 */
#define SERVER_LOG_ERROR(logger) SERVER_LOG_LEVEL(logger, server::LogLevel::ERROR)

/**
 * @brief 使用流式方式将日志级别fatal的日志写入到logger
 */
#define SERVER_LOG_FATAL(logger) SERVER_LOG_LEVEL(logger, server::LogLevel::FATAL)

/**
 * @brief 使用格式化方式将日志级别level的日志写入到logger
 */
#define SERVER_LOG_FMT_LEVEL(logger, level, fmt, ...) \
    if(logger->getLevel() <= level) \
        server::LogEventWrap(server::LogEvent::ptr(new server::LogEvent(logger, level, \
                        __FILE__, __LINE__, 0, time(0), server::GetThreadId()))).getEvent()->format(fmt, __VA_ARGS__)
                		// server::GetFiberId(), time(0), server::Thread::GetName()))).getEvent()->format(fmt, __VA_ARGS__)

/**
 * @brief 使用格式化方式将日志级别debug的日志写入到logger
 */
#define SERVER_LOG_FMT_DEBUG(logger, fmt, ...) SERVER_LOG_FMT_LEVEL(logger, server::LogLevel::DEBUG, fmt, __VA_ARGS__)

/**
 * @brief 使用格式化方式将日志级别info的日志写入到logger
 */
#define SERVER_LOG_FMT_INFO(logger, fmt, ...)  SERVER_LOG_FMT_LEVEL(logger, server::LogLevel::INFO, fmt, __VA_ARGS__)

/**
 * @brief 使用格式化方式将日志级别warn的日志写入到logger
 */
#define SERVER_LOG_FMT_WARN(logger, fmt, ...)  SERVER_LOG_FMT_LEVEL(logger, server::LogLevel::WARN, fmt, __VA_ARGS__)

/**
 * @brief 使用格式化方式将日志级别error的日志写入到logger
 */
#define SERVER_LOG_FMT_ERROR(logger, fmt, ...) SERVER_LOG_FMT_LEVEL(logger, server::LogLevel::ERROR, fmt, __VA_ARGS__)

/**
 * @brief 使用格式化方式将日志级别fatal的日志写入到logger
 */
#define SERVER_LOG_FMT_FATAL(logger, fmt, ...) SERVER_LOG_FMT_LEVEL(logger, server::LogLevel::FATAL, fmt, __VA_ARGS__)

/**
 * @brief 获取主日志器
 */
#define SERVER_LOG_ROOT() server::LoggerMgr::GetInstance()->getRoot()

/**
 * @brief 获取name的日志器
 */
#define SERVER_LOG_NAME(name) server::LoggerMgr::GetInstance()->getLogger(name)

namespace server
{
	class Logger;
	class LogFormatter;
	class LoggerManager;

	/**
	 * @brief 日志级别
	*/
	class LogLevel
	{
	public:
		enum Level
		{
			UNKNOW = 0,	// 
			DEBUG = 1,	// 调试信息
			INFO  = 2,	// 一般信息
			WARN  = 3,	// 警告
			ERROR = 4,	// 错误
			FATAL = 5,	// 致命情况
		};

		/**
		 * @brief 将日志级别转成文本输出
		 * @param[in] level 日志级别
		*/
		static const char* ToString(LogLevel::Level level);

		static LogLevel::Level FromString(const std::string& str);
	};

	/**
	 * @brief 日志事件
	*/
	class LogEvent
	{
	public:
		typedef std::shared_ptr<LogEvent> ptr;

		/**
		 * @brief 构造函数
		 * @param[in] logger 日志器
		 * @param[in] level 日志级别
		 * @param[in] file 文件名
		 * @param[in] line 文件行号
		 * @param[in] elapse 程序启动依赖的耗时(毫秒)
		 * @param[in] thread_id 线程id
		 * @param[in] fiber_id 协程id
		 * @param[in] time 日志事件(秒)
		 * @param[in] thread_name 线程名称
		*/
		LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level
				,const char* file, int32_t line, uint32_t elapse, uint64_t time,uint32_t thread_id);
				//,uint32_t thread_id, uint32_t fiber_id, uint64_t time
				//,const std::string& thread_name);

		/**
		 * @brief 返回文件名
		*/
		const char* getFile() const { return m_file; }

		/**
		 * @brief 返回行号
		*/
		int32_t getLine() const { return m_line; }

		/**
		 * @brief 返回耗时
		*/
		uint32_t getElapse() const { return m_elapse; }

		/**
		 * @brief 返回线程ID
		*/
		uint32_t getThreadId() const { return m_threadId; }

		/**
		 * @brief 返回协程ID
		*/
		uint32_t getFiberId() const { return m_fiberId; }

		/**
		 * @brief 返回时间
		*/
		uint64_t getTime() const { return m_time; }

		/**
		 * @brief 返回线程名称
		*/
		const std::string& getThreadName() const { return m_threadName; }

		/**
		 * @brief 返回日志内容
		*/
		std::string getContent() const { return m_ss.str(); }

		/**
		 * @brief 返回日志器
		*/
		std::shared_ptr<Logger> getLogger() const { return m_logger; }

		/**
		 * @brief 返回日志级别
		 */
		LogLevel::Level getLevel() const { return m_level;}

		/**
		 * @brief 返回日志内容字符串流
		 */
		std::stringstream& getSS() { 
			// std::cout << "LogEvent getSS!" << std::endl;
			return m_ss;}

		/**
		 * @brief 格式化写入日志内容
		*/
		void format(const char* fmt, ...);

		/**
		 * @brief 格式化写入日志内容
		*/
		void format(const char* fmt, va_list al);

	private:
		const char *m_file = nullptr;	// 文件名
		int32_t m_line = 0;				// 行号
		uint32_t m_elapse = 0;			// 程序启动开始到现在的毫秒数
		uint32_t m_threadId = 0;		// 线程 id
		uint32_t m_fiberId = 0;			// 携程 id
		uint64_t m_time;				// 时间戳
		std::string m_threadName;
		std::stringstream m_ss;
		std::shared_ptr<Logger> m_logger;
		// std::string m_content;			//
		LogLevel::Level m_level; 
	};

	/**
	 * @brief 日志事件包装器
	 */
	class LogEventWrap {
	public:

		/**
		 * @brief 构造函数
		 * @param[in] e 日志事件
		 */
		LogEventWrap(LogEvent::ptr e);

		/**
		 * @brief 析构函数
		 */
		~LogEventWrap();

		/**
		 * @brief 获取日志事件
		 */
		LogEvent::ptr getEvent() const { return m_event;}

		/**
		 * @brief 获取日志内容流
		 */
		std::stringstream& getSS();
	private:
		/**
		 * @brief 日志事件
		 */
		LogEvent::ptr m_event;
	};

	/**
	 * @brief 日志格式化
	*/
	class LogFormatter
	{
	public:
		typedef std::shared_ptr<LogFormatter> ptr;

		/**
		 * @brief 构造函数
		 * @param[in] pattern 格式模板
		 * @details 
		 *  %m 消息
		 *  %p 日志级别
		 *  %r 累计毫秒数
		 *  %c 日志名称
		 *  %t 线程id
		 *  %n 换行
		 *  %d 时间
		 *  %f 文件名
		 *  %l 行号
		 *  %T 制表符
		 *  %F 协程id
		 *  %N 线程名称
		 *
		 *  默认格式 "%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"
		 */
		LogFormatter(const std::string& pattern);

		/**
		 * @brief 返回格式化日志文本
		 * @param[in] logger 日志器
		 * @param[in] level 日志级别
		 * @param[in] event 日志事件
		*/
		std::string format(std::shared_ptr<Logger> logger,
						   LogLevel::Level level,
						   LogEvent::ptr event
						   );
		
		std::ostream& format(std::ostream& ofs, 
							std::shared_ptr<Logger> logger, 
							LogLevel::Level level, 
							LogEvent::ptr event
							);
		
	public:
		/**
		 * @brief 日志内容项格式化
		 */
		class FormatItem
		{
		public:
			typedef std::shared_ptr<FormatItem> ptr;

			/**
			 * @brief 析构函数
			*/
			virtual ~FormatItem() {}

			/**
			 * @brief 格式化日志到流
			 * @param[in, out] os 日志输出流
			 * @param[in] logger 日志器
			 * @param[in] level 日志等级
			 * @param[in] event 日志事件
			*/
			virtual void format(std::ostream& os,
									   std::shared_ptr<Logger> logger,
									   LogLevel::Level level,
									   LogEvent::ptr event
									   ) = 0;
		};

		/**
		 * @brief 初始化，解析日志模板
		*/
		void init();

		/**
		 * @brief 是否有错误
		*/
		bool isError() const { return m_error; }

		/**
		 * @brief 返回日志模板
		*/
		const std::string getPattern() const { return m_pattern;}

	private:
		// 日志格式模板
		std::string m_pattern;
		// 日志解析后格式
		std::vector<FormatItem::ptr> m_items;
		// 是否有错误
		bool m_error = false;
	};

	/**
	 * @brief 日志输出目标
	*/
	class LogAppender
	{
	friend class Logger;
	public:
		typedef std::shared_ptr<LogAppender> ptr;

		/**
		 * @brief Destroy the Log Appender object
		 * 
		 */
		virtual ~LogAppender() {}

		/**
		 * @brief 写入日志
		 * @param[in] logger 日志器
		 * @param[in] level 日志级别
		 * @param[in] event 日志事件
		*/
		virtual void log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;

		virtual std::string toYamlString() = 0;
		/**
		 * @brief 更改日志格式器
		*/
		void setFormatter(LogFormatter::ptr val);

		/**
		 * @brief 获取日志格式器
		 */
		LogFormatter::ptr getFormatter();

		/**
		 * @brief 获取日志级别
		*/
		LogLevel::Level getLevel() const { return m_level; }

		/**
		 * @brief 设置日志级别
		*/
		void setLevel(LogLevel::Level val) { m_level = val; }
	protected:
		/// 日志级别
		LogLevel::Level m_level = LogLevel::DEBUG;
		// 日志格式器
		LogFormatter::ptr m_formatter;
		// 是否有自己的日志格式器
		bool m_hasFormatter = false;

		Mutex m_mutex;
	};

	// 输出到控制台
	class StdoutLogAppender : public LogAppender 
	{
	public:
		typedef std::shared_ptr<StdoutLogAppender> ptr;
		void log(std::shared_ptr<Logger> logger, 
					LogLevel::Level level, 
					LogEvent::ptr event) override;
		std::string toYamlString() override;
	// protected:
	// 	LogFormatter::ptr m_formatter;
	};

	// 输出到文件
	class FileLogAppender : public LogAppender 
	{
	public:
		typedef std::shared_ptr<FileLogAppender> ptr;
		FileLogAppender(const std::string& filename);
		void log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override;
		std::string toYamlString() override;
		// 重新打开文件
		/**
		 * @brief 重新打开日志文件
		 * 
		 * @return true 
		 */
		bool reopen();

	private:
		std::string m_filename;
		std::ofstream m_filestream;
		uint64_t m_lastTime = 0;
	};

	class Logger: public std::enable_shared_from_this<Logger>
	{
	friend class LoggerManager;
	public:
		typedef std::shared_ptr<Logger> ptr;

		/**
		 * @brief 构造函数
		 * @param[in] name 日志器名称
		 */
		Logger(const std::string& name = "root");

		/**
		 * @brief 写日志
		 * @param[in] level 日志级别
		 * @param[in] event 日志事件
		 */
		void log(LogLevel::Level level, LogEvent::ptr event);

		void debug(LogEvent::ptr event);
		void info(LogEvent::ptr event);
		void warn(LogEvent::ptr event);
		void error(LogEvent::ptr event);
		void fatal(LogEvent::ptr event);

		/**
		 * @brief 设置日志格式器
		 */
		void setFormatter(LogFormatter::ptr val);

		/**
		 * @brief 设置日志格式模板
		 */
		void setFormatter(const std::string& val);

		LogFormatter::ptr getFormatter();

		/**
		 * @brief 添加日志目标
		 * @param[in] appender 日志目标
		 */
		void addAppender(LogAppender::ptr appender);

		/**
		 * @brief 删除日志目标
		 * @param[in] appender 日志目标
		 */
		void delAppender(LogAppender::ptr appender);

		/**
		 * @brief 清空日志目标
		*/
		void clearAppenders();

		/**
		 * @brief 返回日志级别
		 */
		LogLevel::Level getLevel() const { return m_level;}

		/**
		 * @brief 设置日志级别
		 */
		void setLevel(LogLevel::Level val) { m_level = val;}

		/**
		 * @brief 返回日志名称
		*/
		const std::string& getName() const { return m_name; }

		std::string toYamlString();

	private:
		std::string m_name;							// 日志名称
		LogLevel::Level m_level;					// 日志级别
		std::list<LogAppender::ptr> m_appenders;	// Appender集合
		LogFormatter::ptr m_formatter;
		Logger::ptr m_root;

		Mutex m_mutex;
	};

	/**
	 * @brief 日志器管理类
	*/
	class LoggerManager
	{
	public:
		/**
		 * @brief 构造函数
		*/
		LoggerManager();

		/**
		 * @brief 获取日志器
		 * @param[in] name 日志器名称
		*/
		Logger::ptr getLogger(const std::string& name);

		/**
		 * @brief 初始化
		*/
		void init();

		/**
		 * @brief 返回日志器
		*/
		Logger::ptr getRoot() const { 
			// std::cout << "loggerManager get Root" << std::endl;
			return m_root;}
		
		/**
		 * @brief 将所有的日志器配置转成YAML String
		 */
		std::string toYamlString();

	private:
		// 日志器容器
		std::map<std::string, Logger::ptr> m_loggers;
		// 主日志器
		Logger::ptr m_root;

		Mutex m_mutex;
	};

	// 日志器管理类单例模式
	typedef server::Singleton<LoggerManager> LoggerMgr;
}

#endif