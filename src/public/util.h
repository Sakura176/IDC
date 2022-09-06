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


namespace server
{
	/**
	 * @brief 返回当前线程的ID
	 */
	long GetThreadId();

	template<class T>
	const char* TypeToName() {
		static const char* s_name = abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, nullptr);
		return s_name;
	}

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



