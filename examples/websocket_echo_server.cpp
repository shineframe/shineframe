#include <iostream>
#include "websocket/websocket_server.hpp"

using namespace shine;
using namespace shine::net;
using namespace websocket;

int main(){
    proactor_engine engine;

    shine::string addr = "0.0.0.0:20000";
    bool rc = engine.add_acceptor("websocket_echo_server", addr, [&engine](bool status, connection *conn)->bool{
        if (status)
        {
            websocket::server_peer *conn_handle = new websocket::server_peer;
            conn_handle->set_recv_timeout(0);

            conn_handle->register_recv_callback([](frame_type type, const int8 *data, shine::size_t len, net::connection *conn)->bool{
                
                if (type == websocket::e_text)
                {
                    shine::string response = parser::encode(type, data, len);
                    conn->async_send(response.data(), response.size());
                }
                else if (type == websocket::e_binary)
                {
                    shine::string response = parser::encode(type, data, len);
                    conn->async_send(response.data(), response.size());
                }

                return true;            
            });

            conn_handle->register_close_callback([](net::connection *conn){
                std::cout << "websocket_close_callback:" << conn << std::endl;
            });

            conn_handle->run(conn);
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
