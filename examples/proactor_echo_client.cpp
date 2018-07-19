#include <iostream>
#include "net/proactor_engine.hpp"

using namespace shine;
using namespace shine::net;

int main(){
    std::cout << "connect address: ";
    shine::string addr;
    std::cin >> addr;

    proactor_engine engine;

    bool rc = engine.add_connector("client", addr, [](bool ok, connector *conn){
        std::cout << "connect:" << ok << endl;
        if (ok)
        {
            conn->set_recv_timeout(10000);

            conn->register_recv_callback([](const int8 *data, shine::size_t len, connection *conn)->bool{
                std::cout << "recv_callback len:" << len << " data:" << data << std::endl;
                conn->get_timer_manager()->set_timer(5000, [conn]()->bool{
                    shine::string str = "hello world!";
                    conn->async_send(str.data(), str.size());                 
                    return false;
                });
                return true;
            });

            conn->register_send_callback([](shine::size_t len, connection *conn)->bool{
                return true;
            });

            conn->register_recv_timeout_callback([](connection *conn)->bool{
                std::cout << "recv_timeout_callback:" << endl;
                return true;
            });

            conn->register_send_timeout_callback([](connection *conn)->bool{
                std::cout << "send_timeout_callback:" << endl;
                return false;
            });

            conn->register_close_callback([](connection *conn){
                std::cout << "close_callback:" << endl;
            });

            shine::string data = "hello world";
            conn->async_send(data.data(), data.size());
            conn->async_recv();
        }
    });

    if (rc)
    {
        std::cout << "connect " << addr << "success." << endl;
        engine.run();
    }
    else
    {
        std::cout << "connect " << addr << "failed." << endl;
    }

    return 0;
}