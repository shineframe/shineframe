#include <iostream>
#include <set>
#include <thread>
#include <chrono>
#include "util/tool.hpp"

#include "util/string.hpp"

#include<iostream>
#include <string>
#include<stdio.h>
#include<memory.h>
using namespace std;

using namespace shine;

int main(){

	shine::tool::get_time(shine::tool::get_timestamp());

	shine::string gbk, utf8;
	gbk = "abcdÖÐ¹ú";
	shine::string::gbk_to_utf8(gbk, utf8);
	std::cout << utf8;
	shine::string::utf8_to_gbk(utf8, gbk);
	std::cout << gbk;
	shine::string str = 123.456;
//     str.format("%s,%d", "hello world!", 22334455);
    return 0;
}
