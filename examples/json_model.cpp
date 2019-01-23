#include <iostream>
#include "util/json.hpp"

using namespace shine;

struct B
{
    int a;
    double b;
    std::string c;
    SHINE_JSON_MODEL(B, a, b, c);
};

struct A{
    int a;
    double b;
    std::string c;

    std::map<int, B> d;
    std::list<int> e;
    std::vector<float> f;
    std::deque<double> g;
    std::forward_list<long> h;
    std::set<shine::string> i;

    SHINE_JSON_MODEL(A, a, b, c, d, e, f, g, h, i);
};


int main(){

    A a;
    a.a = 123;
    a.b = 345.567;
    a.c = "hello world!";

    B b;

    b.a = 666;
    b.b = 777.7777;
    b.c = "999999!";

    a.d.emplace(999, b);

    //a.e.emplace_back(123);
    //a.e.emplace_back(345);

    a.f.emplace_back((float)12.34);
    a.f.emplace_back((float)45.67);

    a.g.emplace_back((double)456.789);
    a.g.emplace_back((double)78.9);

    a.h.emplace_front(666);
    a.h.emplace_front(555);

    a.i.emplace("A");
    a.i.emplace("B");
    a.i.emplace("C");

    //将对象a编码成json字符串
    auto a_str = a.json_encode();

    //将json字符串解码成对象，解码后a2与a中数据相同
    A a2;
    a2.json_decode(a_str);

    return 0;
}
