#pragma once

#include <iostream>
#include <chrono>
#include <ctime>
#include <string>
#include <mutex>
#include <map>
#include <unordered_map>
#include <vector>
#include <functional>
#include "../shine_serial/shine_serial.hpp"

using namespace std;

struct shine_rpc_header{
    shine::uint64 sequence = 0;
    SHINE_SERIAL(shine_rpc_header, sequence);
};

struct shine_rpc_func1_request{
    shine_rpc_header header;
    int a = 0;
    int b = 0;
    SHINE_SERIAL(shine_rpc_func1_request, header, a, b);
};

struct shine_rpc_func1_response{
    shine_rpc_header header;
    int sum = 0;
    SHINE_SERIAL(shine_rpc_func1_response, header, sum);
};
