#include <iostream>
#include "shine_serial/shine_serial.hpp"

struct add_request{
    static const shine::size_t type = 100;

    int a = 0;
    int b = 0;
    SHINE_SERIAL(add_request, a, b);
};

struct add_response{
    static const shine::size_t type = 101;

    int total = 0;
    SHINE_SERIAL(add_response, total);
};

struct echo_request{
    static const shine::size_t type = 102;
    std::string message;
    SHINE_SERIAL(echo_request, message);
};

struct echo_response{
    static const shine::size_t type = 103;

    std::string message;
    SHINE_SERIAL(echo_response, message);
};
