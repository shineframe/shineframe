#pragma once

#include "socket.hpp"
#include <csignal>

#if (defined SHINE_OS_WINDOWS)
#include "iocp.hpp"
#else
#include "epoll.hpp"
#endif

