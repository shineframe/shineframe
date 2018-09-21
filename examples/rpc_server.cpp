#include <iostream>
#include <thread>
#include "rpc/rpc_server.hpp"
#include "rpc_protocol.hpp"

using namespace shine;

int main()    {
    net::proactor_engine engine;

    std::pair<socket_t, socket_t> pair;
    net::socket::create_socketpair(pair);
    rpc::pipe_server server(engine, pair.first);

    rpc::pipe_client client(pair.second);

    std::thread t([&client, &engine]{
        {
            add_request req;
            add_response rsp;
            bool rc = false;

            for (int i = 0; i < 100; i++)
            {
                req.a = i * i;
                req.b = i + 1;
                client.call(req.type, req, rsp, rc);
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
            client.call(req.type, req, rsp, rc);
            std::cout << i << "  " << rc << " result:" << rsp.message << std::endl;
        }

    }
    });


//     new std::thread([&engine, &server]{
        server.register_rpc_handle(add_request::type, [](const int8 *data, shine::size_t len, package_t &header, shine::string &rsp_data)->bool{

            add_request req;
            if (!req.shine_serial_decode(data, len))
                return false;

            add_response rsp;
            rsp.total = req.a + req.b;

            header.identify = rsp.type;
            rsp_data = rsp.shine_serial_encode();
            return true;
        });

        server.register_rpc_handle(echo_request::type, [](const int8 *data, shine::size_t len, package_t &header, shine::string &rsp_data)->bool{

            echo_request req;
            if (!req.shine_serial_decode(data, len))
                return false;

            echo_response rsp;
            rsp.message = "PONG";

            header.identify = rsp.type;
            rsp_data = rsp.shine_serial_encode();
            return true;
        });

//     });

    engine.run();

    return 0;
}

