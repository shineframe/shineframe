#include <iostream>
#include <set>
#include <thread>
#include <chrono>

#include "redis/redis.hpp"

using namespace shine;
using namespace shine::net;

int main(){
    shine::redis::sync redis;
    redis.set_addr("172.10.4.19:6000");
    redis.set_addr("push_dev.ineice.cn:7000");
    redis.set_recv_timeout(3000);
    redis.set_auth("1qazxsw23edc");

    shine::string str;
    std::vector<shine::string> arr;
    std::set<shine::string> set;
    std::map<shine::string, shine::string> map;

    //     redis.SET("library", "redis");
    //     redis.GET("library", str);
    //     std::cout << str << std::endl;
    // 
    // 
    //     redis.SADD("names", "a", "b", "c", "e");
    //     redis.SMEMBERS("names", set);
    //     for (auto &iter : set){
    //         std::cout << iter << std::endl;
    //     }

    redis.KEYS("*dbec6@*>m>*", arr);
    for (auto &iter : arr){
        redis.DEL(iter);
    }

    redis.KEYS("*dbec6@*>dmq>*", arr);
    for (auto &iter : arr){
        redis.DEL(iter);
    }

    redis.KEYS("c3311f70431d2ce6>m>*", arr);
    for (auto &iter : arr){
        redis.DEL(iter);
        //         std::cout << iter << std::endl;
    }

    redis.KEYS("c3311f70431d2ce6>dmq>*", arr);
    for (auto &iter : arr){
        redis.DEL(iter);
        //         std::cout << iter << std::endl;
    }

    //     redis.HSET("members", "a", "A");
    //     redis.HSET("members", "b", "B");
    //     redis.HSET("members", "c", "C");
    //     redis.HGETALL("members", map);
    //     for (auto &iter : map){
    //         std::cout << iter.first << ":" << iter.second << std::endl;
    //     }

    return 0;
}
