#include <iostream>
#include <thread>
#include "concurrent_queue/concurrent_queue.hpp"
#include "shine_serial/shine_serial.hpp"

using namespace shine;

struct stru_1 {
    int a = 0;
    SHINE_SERIAL(stru_1, a);
};

int main()    {
    concurrent_queue<stru_1> q;
    stru_1 obj;

    for (int i = 0; i != 123; ++i)
    {
        obj.a = i;
        q.push(obj);
    }

    for (int i = 0; i != 123; ++i) {
        q.pop(obj); 
        std::cout << (obj.a == i) << std::endl;
    }
    return 0;
}

