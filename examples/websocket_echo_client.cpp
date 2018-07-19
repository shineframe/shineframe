#include <iostream>
#include "http/http_client.hpp"

using namespace shine;
using namespace shine::net;
using namespace http;

int main(){
    sync_client clinet;
    clinet.set_recv_timeout(3000);
    clinet.get_request().set_host("www.baidu.com");
    clinet.get_request().set_method(http::method::get);
    clinet.get_request().set_url("/");

    if (clinet.call())
    {
        shine::string tmp;
        clinet.get_response().encode(tmp);
        std::cout << tmp << std::endl;
    }
    else
    {
        std::cout << "" << std::endl;
    }

    return 0;
}
