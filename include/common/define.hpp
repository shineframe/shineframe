#pragma once

#ifdef _WIN32
//define something for Windows (32-bit and 64-bit, this part is common)
#define SHINE_OS_WINDOWS
#define SHINE_OS "WINDOWS"
#ifdef _WIN64
//define something for Windows (64-bit only)
#else
//define something for Windows (32-bit only)
#endif
#elif __APPLE__
#define SHINE_OS_APPLE
#include "TargetConditionals.h"
#if TARGET_IPHONE_SIMULATOR
// iOS Simulator
#define SHINE_OS_IOS_SIMULATOR
#define SHINE_OS "IOS_SIMULATOR"
#elif TARGET_OS_IPHONE
// iOS device
#define SHINE_OS_IOS_PHONE
#define SHINE_OS "IOS_PHONE"
#elif TARGET_OS_MAC
// Other kinds of Mac OS
#define SHINE_OS_MAC
#define SHINE_OS "MAC"
#else
#   error "Unknown Apple platform"
#endif
#elif __ANDROID__
// android
#define SHINE_OS_ANDROID
#define SHINE_OS_LINUX
#define SHINE_OS "ANDROID"
#elif __linux__
// linux
#define SHINE_OS_LINUX
#define SHINE_OS "LINUX"
#elif __unix__ // all unices not caught above
// Unix
#define SHINE_OS_UNIX
#define SHINE_OS "UNIX"
#elif defined(_POSIX_VERSION)
// POSIX
#define SHINE_OS_POSIX "POSIX"
#define SHINE_OS "POSIX"
#else
#   error "Unknown compiler"
#endif

#include <iostream>
namespace shine
{
    typedef bool Bool;
    typedef char int8;
    typedef unsigned char  uint8;
    typedef short  int16;
    typedef unsigned short  uint16;
    typedef int  int32;
    typedef unsigned int  uint32;
    typedef long  Long;
    typedef unsigned long  uLong;
    typedef long long  int64;
    typedef unsigned long long  uint64;
    typedef float Float;
    typedef double Double;
    typedef long double LDouble;
    typedef std::size_t size_t;
    typedef size_t size_type;

    struct iovec_t {
        const int8 *data;
        size_t size;
    };
}

#include "macros.hpp"

