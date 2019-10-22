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

// using namespace std;

namespace shine
{
	struct time_t
	{
		int64 timestamp;
		int year;
		int month;
		int day;
		int hour;
		int mintue;
		int second;
		int milliseconds;
		int wday;
		int yday;
		char str[32];
	};

    class tool
    {
    public:
        static inline int64 get_timestamp()
        {
			std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> tp = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
			auto tmp = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
			std::time_t timestamp = tmp.count();
			return timestamp;
		}

		static inline shine::time_t get_time()
		{
			shine::time_t ret;
			ret.timestamp = get_timestamp();
			std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> tp(std::chrono::milliseconds(ret.timestamp + 8 * 60 * 60 * 1000));

			auto tt = std::chrono::system_clock::to_time_t(tp);
			std::tm* now = std::gmtime(&tt);

			ret.year = now->tm_year + 1900;
			ret.month = now->tm_mon + 1;
			ret.day = now->tm_mday;
			ret.yday = now->tm_yday;
			ret.wday = now->tm_wday;
			ret.hour = now->tm_hour;
			ret.mintue = now->tm_min;
			ret.second = now->tm_sec;
			ret.milliseconds = ret.timestamp % 1000;

			sprintf(ret.str, "%04d-%02d-%02d %02d:%02d:%02d:%03d", ret.year, ret.month, ret.day, ret.hour
			, ret.mintue, ret.second, ret.milliseconds);
			return std::move(ret);
		}

		static std::tm* get_time(int64 timestamp)
		{
			int64 milli = timestamp + (int64)8 * 60 * 60 * 1000;//此处转化为东八区北京时间，如果是其它时区需要按需求修改
			auto mTime = std::chrono::milliseconds(milli);
			auto tp = std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>(mTime);
			auto tt = std::chrono::system_clock::to_time_t(tp);
			std::tm* now = std::gmtime(&tt);
			return now;
		}

        static inline std::string get_datetime(bool ms = false)
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

namespace shine
{
    namespace serial
    {
        enum{
            e_bool = 0,
            e_byte = 1,
            e_integer = 2,
            e_float = 3,
            e_double = 4,
            e_bytes = 5,
            e_array = 6,
            e_set = 7,
            e_map = 8,
            e_struct = 9,
        };

        template<typename T>
        inline void encode_size(std::string &buf, T size){
            do {
                shine::int8 ch = size & ((1 << 7) - 1);
                if (size >>= 7)
                    ch |= 0x80;

                buf += ch;
            } while (size);
        }

        template<typename T>
        inline shine::size_t decode_size(T &val, const shine::int8 *data, const shine::size_t len){
            if (len < 1)
                return 0;

            val = 0;
            const shine::int8 *p = data;
            shine::size_t i = 0;

            for (;;)
            {
                val |= (shine::size_t)(p[i] & 0x7F) << (7 * i);
                if (p[i++] & 0x80) {
                    if (i >= len)
                        return 0;
                }
                else {
                    break;
                }
            }

            return i;
        }
    }

    struct package_t{
        size_t length = 0;
        size_t identify = 0;

        inline std::string encode(){
            std::string ret;

            serial::encode_size(ret, length);
            serial::encode_size(ret, identify);

            return std::move(ret);
        }

        inline size_t decode(const int8 *data, const size_t len){
            size_t cost_len = serial::decode_size(length, data, len);

            if (cost_len == 0)
                return 0;

            size_t cost_len2 = serial::decode_size(identify, data + cost_len, len - cost_len);

            if (cost_len2 == 0)
                return 0;

            return cost_len + cost_len2;
        }
    };

}
