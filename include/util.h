/**
 * @file util.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-08-14
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef __SERVER_UTIL_H__
#define __SERVER_UTIL_H__

#include <cxxabi.h>
#include <stdio.h>
#include <stdint.h>
#include <boost/lexical_cast.hpp>

namespace server
{
	/**
	 * @brief 返回当前线程的ID
	 */
	long GetThreadId();

	std::string Time2Str(time_t ts = time(0), const std::string &format = "%Y-%m-%d %H:%M:%S");
	time_t Str2Time(const char* str, const char* format = "%Y-%m-%d %H:%M:%S");

	template<class T>
	const char* TypeToName() {
		static const char* s_name = abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, nullptr);
		return s_name;
	}

	template<class V, class Map, class K>
	V GetParamValue(const Map& m, const K& k, const V& def = V())
	{
		auto it = m.find(k);
		if(it == m.end())
		{
			return def;
		}
		try
		{
			return boost::lexical_cast<V>(it->second);
		}
		catch(...)
		{ }

		return def;
	}

	template<class V, class Map, class K>
	bool CheckGetParamValue(const Map& m, const K& k, V& v) 
	{
		auto it = m.find(k);
		if(it == m.end()) 
		{
			return false;
		}
		try 
		{
			v = boost::lexical_cast<V>(it->second);
			return true;
		} 
		catch (...) 
		{		}

		return false;
	}

	template<class T>
	void nop(T*) {}

	class StringUtil
	{
	public:
		static std::string Formatv(const char *fmt, va_list ap);
	};

	class TypeUtil
	{
	public:
		static int8_t ToChar(const std::string &str);
		static int64_t Atoi(const std::string& str);
		static double Atof(const std::string& str);
		static int8_t ToChar(const char* str);
		static int64_t Atoi(const char* str);
		static double Atof(const char* str);
	};

	class Nocopyable
	{
	public:
		Nocopyable() = default;

		~Nocopyable() = default;

		Nocopyable(const Nocopyable&) = delete;
		
		Nocopyable& operator=(const Nocopyable&) = delete;
	};
}

#endif



