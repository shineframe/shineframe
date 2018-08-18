#include <iostream>
#include <set>
#include <thread>
#include <chrono>

#include "util/string.hpp"

using namespace shine;

int main(){

    shine::string str = 123.456;
    str.format("%s,%d", "hello world!", 22334455);
    return 0;
}
