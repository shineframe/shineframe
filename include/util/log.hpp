
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

const char* const log_level_desc[] = { "D", "I", "W", "E", "F" };
#define log_buf_size  2 * 1024 * 1024

namespace shine
{
    class log
    {
    public:
        enum { e_debug = 0, e_info = 1, e_warning = 2, e_error = 3, e_fatal = 4 };
        enum { e_file = 1, e_console = 2, e_socket = 4 };
        enum { e_no_split = 0, e_year = 4, e_month = 7, e_day = 10, e_hour = 13 };
    public:
        void init(const std::string &filename, int filter_level = e_info, int split = e_day) {
            _filename = filename;
            _filter_level = filter_level;
            _split = split;
        }

    public:
        void write(int Level, int target, const char *fmt, ...)
        {
            if (Level < _filter_level)
                return;

            std::unique_lock<std::recursive_mutex> lock(_mutex);
            std::string datetime = tool::get_datetime(true);

            auto len = SHINE_SNPRINTF(_buf, log_buf_size, "%s [%04X] %s ", datetime.c_str() + 11, (int)GETTID(), log_level_desc[Level]);

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
                datetime[_split] = '\0';
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
        int _split;
        int _filter_level;
        char _buf[log_buf_size];
    };

    
}

