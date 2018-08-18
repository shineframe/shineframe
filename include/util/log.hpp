 /**
 *****************************************************************************
 *
 *@note shineframe开发框架 https://github.com/shineframe/shineframe
 *
 *@file log.hpp
 *
 *@brief 日志
 *
 *@todo 
 *
 *@author sunjian 39215174@qq.com
 *
 *@version 1.0
 *
 *@date 2018/6/14 
 *****************************************************************************
 */
#pragma once

#include "../common/define.hpp"
#include "tool.hpp"
#include <stdarg.h>
#include <mutex>

#if (defined SHINE_OS_WINDOWS)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <direct.h>
#define GETCWD _getcwd
#define GETPID GetCurrentProcessId
#define GETTID GetCurrentThreadId
#else
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#define GETCWD getcwd
#define GETPID getpid
#define GETTID pthread_self

#endif

using namespace std;

#if (defined SHINE_OS_WINDOWS)
#pragma warning(disable:4996)
#endif

const char* const log_level_desc[] = { "debug", "info", "warning", "error", "fatal" };
#define log_buf_size  1024 * 10

namespace shine
{
    class log
    {
    public:
        enum { e_debug, e_info, e_warning, e_error, e_fatal };
        enum { e_file = 1, e_console = 2, e_socket = 4 };

    public:
        void init(const std::string &filename, int filter_level = e_info) {
            _filename = filename;
            _filter_level = filter_level;
        }

    public:
        void write(int Level, int target, const char *fmt, ...)
        {
            if (Level < _filter_level)
                return;

            std::unique_lock<std::recursive_mutex> lock(_mutex);
            std::string datetime = tool::get_datetime(true);

            auto len = SHINE_SNPRINTF(_buf, log_buf_size, "%s [%04X] %s ", datetime.c_str(), (int)GETTID(), log_level_desc[Level]);

            va_list valist;
            va_start(valist, fmt);
            len += SHINE_VSNPRINTF((char*)_buf + len, log_buf_size - len - 2, fmt, valist);
            va_end(valist);

            _buf[len++] = '\n';

            if (target & e_console)
            {
                _buf[len] = '\0';
                puts(_buf);
            }

            if (target & e_file)
            {
                datetime[10] = '\0';
                char path[256];
                auto path_len = SHINE_SNPRINTF(path, sizeof(path), "%s_%s.log", _filename.c_str(), datetime.c_str());
                
                path[path_len] = '\0';

                FILE *f = fopen(path, "a");
                if (f)
                {
                    fwrite(_buf, len, 1, f);
                    fclose(f);
                }
            }

            if (target & e_socket)
            {
                //not impl
            }
        }

    private:
        std::recursive_mutex _mutex;
        std::string _filename;
        int _filter_level;
        char _buf[log_buf_size];
    };
}

