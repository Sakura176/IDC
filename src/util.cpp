#include "../include/util.h"
#include "../include/log.h"
#include <execinfo.h>
#include <sys/time.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <sys/syscall.h>

namespace server
{
	static server::Logger::ptr g_logger = SERVER_LOG_NAME("system");
	long GetThreadId()
	{
		return syscall(SYS_gettid);
	}

	std::string Time2Str(time_t ts, const std::string& format)
	{
		struct tm tm;
		localtime_r(&ts, &tm);
		char buf[64];
		strftime(buf, sizeof(buf), format.c_str(), &tm);
		return buf;
	}

	time_t Str2Time(const char* str, const char* format) 
	{
		struct tm t;
		memset(&t, 0, sizeof(t));
		if(!strptime(str, format, &t)) 
		{
			return 0;
		}
		return mktime(&t);
	}

	/**
	 * @brief 
	 * 
	 * @param str 
	 * @return std::string 
	 */
	static std::string demangle(const char *str) {
		size_t size = 0;
		int status  = 0;
		std::string rt;
		rt.resize(256);
		if (1 == sscanf(str, "%*[^(]%*[^_]%255[^)+]", &rt[0])) {
			char *v = abi::__cxa_demangle(&rt[0], nullptr, &size, &status);
			if (v) {
				std::string result(v);
				free(v);
				return result;
			}
		}
		if (1 == sscanf(str, "%255s", &rt[0])) {
			return rt;
		}
		return str;
	}

	void Backtrace(std::vector<std::string> &bt, int size, int skip) {
		void **array = (void **)malloc((sizeof(void *) * size));
		size_t s     = ::backtrace(array, size);

		char **strings = backtrace_symbols(array, s);
		if (strings == NULL) {
			SERVER_LOG_ERROR(g_logger) << "backtrace_synbols error";
			return;
		}

		for (size_t i = skip; i < s; ++i) {
			bt.push_back(demangle(strings[i]));
		}

		free(strings);
		free(array);
	}

	std::string BacktraceToString(int size, int skip, const std::string &prefix) {
		std::vector<std::string> bt;
		Backtrace(bt, size, skip);
		std::stringstream ss;
		for (size_t i = 0; i < bt.size(); ++i) {
			ss << prefix << bt[i] << std::endl;
		}
		return ss.str();
	}

	uint64_t GetCurrentMS()
	{
		struct timeval tv;
		gettimeofday(&tv, NULL);
		return tv.tv_sec * 1000ul + tv.tv_usec / 1000;
	}

	uint64_t GetCurrentUS()
	{
		struct timeval tv;
		gettimeofday(&tv, NULL);
		return tv.tv_sec * 1000 * 1000ul + tv.tv_usec;
	}

	int8_t  TypeUtil::ToChar(const std::string& str) 
	{
		if(str.empty()) 
		{
			return 0;
		}
		return *str.begin();
	}

	int64_t TypeUtil::Atoi(const std::string& str) 
	{
		if(str.empty()) 
		{
			return 0;
		}
		return strtoull(str.c_str(), nullptr, 10);
	}

	double  TypeUtil::Atof(const std::string& str) 
	{
		if(str.empty()) 
		{
			return 0;
		}
		return atof(str.c_str());
	}

	int8_t  TypeUtil::ToChar(const char* str) 
	{
		if(str == nullptr) 
		{
			return 0;
		}
		return str[0];
	}

	int64_t TypeUtil::Atoi(const char* str) 
	{
		if(str == nullptr) 
		{
			return 0;
		}
		return strtoull(str, nullptr, 10);
	}

	double  TypeUtil::Atof(const char* str) 
	{
		if(str == nullptr) 
		{
			return 0;
		}
		return atof(str);
	}

	std::string StringUtil::Formatv(const char* fmt, va_list ap)
	{
		char *buf = nullptr;
		auto len = vasprintf(&buf, fmt, ap);
		if(len == -1)
		{
			return "";
		}
		std::string ret(buf, len);
		free(buf);
		return ret;
	}
}