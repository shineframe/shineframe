
#pragma once

#include <iostream>
#include <chrono>
#include <ctime>
#include <string>
#include "../common/define.hpp"

#if (defined SHINE_OS_WINDOWS)
#include <winsock2.h>
# define SHINE_STDCALL __stdcall
# define SHINE_DLL_HANDLE HINSTANCE
# define SHINE_DLL_FARPROC FARPROC
#else
#include <dlfcn.h>
# define SHINE_STDCALL 
# define SHINE_DLL_HANDLE void*
# define SHINE_DLL_FARPROC void*
#endif

#include "../net/socket.hpp"

using namespace std;

namespace shine
{
    class dll
    {
    public:
        static SHINE_DLL_HANDLE open(const int8 *path)
        {
            SHINE_DLL_HANDLE handle;

#if defined(SHINE_OS_WINDOWS)
            handle = LoadLibrary(path);
#else
            handle = dlopen(path, RTLD_LOCAL | RTLD_LAZY);
#endif
            return handle;
        }

        static void close(SHINE_DLL_HANDLE handle)
        {
#if defined(SHINE_OS_WINDOWS)
            FreeLibrary(handle);
#else 
            dlclose(handle);
#endif
        }

        static SHINE_DLL_FARPROC sym(SHINE_DLL_HANDLE handle, const int8 *name)
        {
#if defined(SHINE_OS_WINDOWS)
            return GetProcAddress(handle, name);
#else 
            return dlsym(handle, name);
#endif
        }

        static int32 get_error() {
#ifdef SHINE_OS_WINDOWS
            return WSAGetLastError();
#else
            return errno;
#endif
        }
    };
}

