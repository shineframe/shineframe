#pragma once

#include "socket.hpp"
#include <csignal>

#if (defined SHINE_OS_WINDOWS)
#include "iocp.hpp"
#elif (defined SHINE_OS_LINUX)
#include "epoll.hpp"
#elif (defined SHINE_OS_UNIX || defined SHINE_OS_APPLE)
#include "kqueue.hpp"
#endif

