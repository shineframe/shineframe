#include <iostream>
#include <thread>
#include "rpc/rpc_client.hpp"
#include "rpc_protocol.hpp"

using namespace shine;

int main(){
    net::proactor_engine engine;
    rpc::client client("172.10.4.19:7000");

    echo_request ss;

    std::thread t([&client, &engine]{
        {
            add_request req;
            add_response rsp;
            bool rc = false;

            for (int i = 0; i < 100; i++)
            {
                req.a = i * i;
                req.b = i + 1;
                client.sync_call(req.type, req, rsp, rc);
                std::cout << i << "  " << rc << " result:" << rsp.total << std::endl;
            }   

        }

        {
            echo_request req;
            echo_response rsp;
            bool rc = false;

            for (int i = 0; i < 100; i++)
            {
                req.message = "PING";
                client.sync_call(req.type, req, rsp, rc);
                std::cout << i << "  " << rc << " result:" << rsp.message << std::endl;
            }

        }
    });

    engine.run();
    return 0;
}
