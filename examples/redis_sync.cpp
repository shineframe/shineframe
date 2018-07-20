#include <iostream>
#include <set>
#include <vector>
#include <map>
#include "redis/redis.hpp"

using namespace shine;
using namespace shine::net;

int main(){
    shine::redis::sync redis;
    redis.set_addr("127.0.0.1:6379");
    redis.set_recv_timeout(3000);
    redis.set_auth("password");

    shine::string str;
    std::vector<shine::string> arr;
    std::set<shine::string> set;
    std::map<shine::string, shine::string> map;

    redis.SET("library", "redis");
    redis.GET("library", str);
    std::cout << str << std::endl;


    redis.SADD("names", "a", "b", "c", "e");
    redis.SMEMBERS("names", set);
    for (auto &iter : set){
        std::cout << iter << std::endl;
    }

    redis.KEYS("*", arr);
    for (auto &iter : arr){
        std::cout << iter << std::endl;
    }

    redis.HSET("members", "a", "A");
    redis.HSET("members", "b", "B");
    redis.HSET("members", "c", "C");
    redis.HGETALL("members", map);
    for (auto &iter : map){
        std::cout << iter.first << ":" << iter.second << std::endl;
    }

    return 0;
}
