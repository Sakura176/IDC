#include "../include/util.h"
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