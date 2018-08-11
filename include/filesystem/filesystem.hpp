#pragma once
#include <common/define.hpp>
#if (defined SHINE_OS_WINDOWS)
#include "fs_windows.hpp"
#else
#include "fs_linux.hpp"
#endif

#include "file.hpp"

