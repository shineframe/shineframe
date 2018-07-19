#include <iostream>
#include "net/proactor_engine.hpp"

using namespace shine;
using namespace shine::net;

int main(){

    std::cout << "bind address: ";
    shine::string addr;
    std::cin >> addr;

    proactor_engine engine;
    bool rc = engine.add_acceptor("echo_server", addr, [&engine](bool status, connection *conn)->bool{
        if (status)
        {
            conn->set_recv_timeout(0);

            conn->register_recv_callback([](const int8 *data, shine::size_t len, connection *conn)->bool{
                conn->async_send(data, len);
                return true;
            });

            conn->async_recv();
        }

        return true;
    });

    if (rc)
    {
        std::cout << "bind " << addr << "success." << endl;
        engine.run();
    }
    else
    {
        std::cout << "bind " << addr << "failed." << endl;
    }

    return 0;
}