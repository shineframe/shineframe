#include <iostream>
#include <thread>
#include "rpc/rpc_server.hpp"
#include "rpc_protocol.hpp"

using namespace shine;

int main()    {
    net::proactor_engine engine;
    rpc::server server(engine, "", "0.0.0.0:7000");

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

        server.run();
//     });

    engine.run();

    return 0;
}

