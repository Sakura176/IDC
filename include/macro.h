/**
 * @file macro.h
 * @brief 常用宏的封装
 * @author SERVER.yin
 * @email 564628276@qq.com
 * @date 2019-06-01
 * @copyright Copyright (c) 2019年 SERVER.yin All rights reserved (www.SERVER.top)
 */
#ifndef __SERVER_MACRO_H__
#define __SERVER_MACRO_H__

#include <string.h>
#include <assert.h>
#include "log.h"
#include "util.h"

#if defined __GNUC__ || defined __llvm__
/// LIKCLY 宏的封装, 告诉编译器优化,条件大概率成立
#   define SERVER_LIKELY(x)       __builtin_expect(!!(x), 1)
/// LIKCLY 宏的封装, 告诉编译器优化,条件大概率不成立
#   define SERVER_UNLIKELY(x)     __builtin_expect(!!(x), 0)
#else
#   define SERVER_LIKELY(x)      (x)
#   define SERVER_UNLIKELY(x)      (x)
#endif

/// 断言宏封装
#define SERVER_ASSERT(x) \
    if(SERVER_UNLIKELY(!(x))) { \
        SERVER_LOG_ERROR(SERVER_LOG_ROOT()) << "ASSERTION: " #x \
            << "\nbacktrace:\n" \
            << server::BacktraceToString(100, 2, "    "); \
        assert(x); \
    }

/// 断言宏封装
#define SERVER_ASSERT2(x, w) \
    if(SERVER_UNLIKELY(!(x))) { \
        SERVER_LOG_ERROR(SERVER_LOG_ROOT()) << "ASSERTION: " #x \
            << "\n" << w \
            << "\nbacktrace:\n" \
            << server::BacktraceToString(100, 2, "    "); \
        assert(x); \
    }

#endif
