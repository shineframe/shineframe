#pragma once

#include <iostream>
#include <chrono>
#include <ctime>
#include <string>
#include "../common/define.hpp"

#if (defined SHINE_OS_WINDOWS)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <WinSock2.h>

#define SHINE_SNPRINTF(buf, size, fmt, ...) _snprintf_s(buf, size, size, fmt, __VA_ARGS__)
#define SHINE_VSNPRINTF(buf, size, fmt, ...) vsnprintf_s(buf, size, size, fmt, __VA_ARGS__)
#else
#include <stdio.h> 
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>

#define SHINE_SNPRINTF snprintf
#define SHINE_VSNPRINTF vsnprintf

#endif

using namespace std;

namespace shine
{
    class tool
    {
    public:
        static uint64 get_timestamp()
        {
            auto now = std::chrono::high_resolution_clock::now();
            return std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
        }

        static std::string get_datetime(bool ms = false)
        {
            struct timeval	tv;
            struct tm	stime;

#if (defined SHINE_OS_WINDOWS)
            {
                SYSTEMTIME	stNow;
                GetLocalTime(&stNow);
                tv.tv_usec = stNow.wMilliseconds * 1000;
                stime.tm_year = stNow.wYear - 1900;
                stime.tm_mon = stNow.wMonth - 1;
                stime.tm_mday = stNow.wDay;
                stime.tm_hour = stNow.wHour;
                stime.tm_min = stNow.wMinute;
                stime.tm_sec = stNow.wSecond;
            }
#else
            gettimeofday(&tv, NULL);
            localtime_r(&(tv.tv_sec), &stime);
#endif

            int8 buf[32];
            std::size_t len = strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &stime);

            if (ms)
                SHINE_SNPRINTF((char *)buf + len, sizeof(buf) - len, ":%06ld", tv.tv_usec);

            return buf;
        }

    };
}

