#include <iostream>
#include "http/http_server.hpp"

using namespace shine;
using namespace shine::net;
using namespace http;

int main(){
    proactor_engine engine;

    shine::string addr = "0.0.0.0:38300";

	http::server_peer_multi_thread::setup(engine);
    bool rc = engine.add_acceptor("http_base_server", addr, [&engine](bool status, connection *conn)->bool{
        if (status)
        {
            http::server_peer_multi_thread *conn_handle = new http::server_peer_multi_thread;

                conn_handle->set_recv_timeout(15000);
                conn_handle->register_url_handle("/", [](const http::request &request, http::response &response)->bool{
					http::entry_t allow;
					allow.set_key("Access-Control-Allow-Origin");
					allow.set_value("*");
					response.add_entry(allow);
                    response.set_version(request.get_version());
                    response.set_status_code(200);
                    response.set_body("hello shineframe!");
					std::cout << "handle thread: " << std::this_thread::get_id() << std::endl;
                    return true;
                });

                conn_handle->register_url_handle("/api/hello", [](const http::request &request, http::response &response)->bool{
                    response.set_version(request.get_version());
                    response.set_status_code(200);

                    shine::string body = "hello api!\r\n\r\nparameters:\r\n";
                    for (auto pa : request.get_url_parameters())
                    {
                        body += pa.first + "=" + pa.second + "\r\n";
                    }

                    response.set_body(body);
                    return true;
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
