#include <iostream>
#include <set>
#include <thread>
#include <chrono>

#include "redis/redis.hpp"

using namespace shine;
using namespace shine::net;

int main(){

    shine::string str = 123.456;
    str.format("%s,%d", "hello world!", 22334455);
    return 0;
}
