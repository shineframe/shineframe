#pragma once

#include <iostream>
#include <chrono>
#include <ctime>
#include <string>
#include <mutex>
#include <map>
#include <unordered_map>
#include <vector>
#include <functional>
#include "../common/define.hpp"
#include "../util/pool.hpp"
#include "../net/proactor_engine.hpp"
#include "../net/socket.hpp"
#include "redis_parser.hpp"

#if (defined SHINE_OS_WINDOWS)
#else
#endif

using namespace std;

namespace shine
{
    namespace redis
    {

        class client_base{
        public:
            client_base()
            {
            }
            virtual ~client_base(){

            }

            bool recv_callback(const int8 *data, shine::size_t len) {
                get_buf().append(data, len);
                return decode();
            }

            bool decode() {
                for (;;)
                {
                    if (get_buf().size() == get_buf_pos())
                    {
                        get_buf().clear();
                        get_buf_pos() = 0;
                        return true;
                    }

                DECODE:
                    if (get_decode_step() == e_decode_header)
                    {
                        set_current_data(&get_response().get_data());

                        int32 rc = parse_header();
                        if (rc == decode_result::not_enough_header)
                            return true;
                        else if (rc == decode_result::parse_error)
                            return false;
                    }

                    if (get_decode_step() == e_decode_body)
                    {
                        if (get_current_data()->get_type() == e_type_bulk)
                        {
                            int32 rc = parse_bulk();
                            if (rc == decode_result::not_enough_total)
                                return true;
                            else if (rc == decode_result::parse_error)
                                return false;
                            else{
                                data_ptr_t parent = get_current_data()->get_parent();
                                if (parent)
                                {
                                    parent->get_clients()->push_back(get_current_data());
                                    get_current_data() = parent;
                                    goto DECODE;
                                }
                                else
                                {
                                    get_current_data() = nullptr;
                                    set_decode_step(e_decode_done);
                                }
                            }
                        }
                        else if (get_current_data()->get_type() == e_type_multi_bulk)
                        {
                            int32 rc = parse_multi_bulk();
                            if (rc == decode_result::not_enough_total)
                                return true;
                            else if (rc == decode_result::parse_error)
                                return false;
                            else{
                                data_ptr_t parent = get_current_data()->get_parent();
                                if (parent)
                                {
                                    parent->get_clients()->push_back(get_current_data());
                                    get_current_data() = parent;
                                    goto DECODE;
                                }
                                else
                                {
                                    get_current_data() = nullptr;
                                    set_decode_step(e_decode_done);
                                }
                            }
                          }
                        else
                        {
                            return false;
                        }
                    }

                    if (get_decode_step() == e_decode_done)
                    {
                        get_buf().erase(0, get_buf_pos());
                        get_buf_pos() = 0;

                        if (get_mode() == e_mode_sync)
                        {
                            return true;
                        }
                        else
                        {

                        }

                        set_decode_step(e_decode_header);
                    }
                }

                return true;
            }

            int32 parse_header(){
                auto pos = get_buf().find("\r\n", get_buf_pos());
                if (pos == string::npos)
                    return decode_result::not_enough_header;

                reply_data &header = get_response().get_data();

                header.clear();

                int8 ch = get_buf()[get_buf_pos()++];
                header.get_value().assign(get_buf().data() + get_buf_pos(), pos - get_buf_pos());
                set_buf_pos(pos + 2);

                if (ch == '+')
                {
                    header.set_type(e_type_ok);
                    set_decode_step(e_decode_done);
                }
                else if (ch == '-')
                {
                    header.set_type(e_type_error);
                    set_decode_step(e_decode_done);
                }
                else if (ch == ':')
                {
                    header.set_type(e_type_integer);
                    set_decode_step(e_decode_done);
                }
                else if (ch == '$')
                {
                    header.set_type(e_type_bulk);
                    set_decode_step(e_decode_body);
                }
                else if (ch == '*')
                {
                    header.set_type(e_type_multi_bulk);
                    set_decode_step(e_decode_body);
                }
                else
                {
                    set_decode_step(e_decode_header);
                    return decode_result::parse_error;
                }

                return decode_result::success;
            }

            int32 parse_bulk(){
                uint64 cost_len = 0;
                auto rc = parse_bulk_impl(get_current_data(), cost_len);
                if (rc == decode_result::success)
                {
                    set_buf_pos(get_buf_pos() + cost_len);
//                     set_decode_step(e_decode_done);
                }

                return rc;
            }

            int32 parse_bulk_impl(data_ptr_t parent, uint64 &cost_len){
                if (parent->get_value() == "-1")
                {
                    cost_len = 0;
                    return decode_result::success;
                }

                uint64 len = std::stoull(parent->get_value());
                if (get_buf().size() - get_buf_pos() - 2 >= len)
                {
                    if (!parent->get_clients())
                        parent->get_clients() = std::make_shared<array_data_t>();

                    data_ptr_t data = reply_data_pool.take();
                    data->clear();
                    data->set_parent(parent);
                    data->get_value().assign(get_buf().data() + get_buf_pos() + cost_len, len);
                    parent->get_clients()->push_back(std::move(data));
                    cost_len = cost_len + len + 2;
                    return decode_result::success;
                }
                else
                {
                    set_current_data(parent);
                    return decode_result::not_enough_total;
                }
            }

            int32 parse_multi_bulk(){
                uint64 cost_len = 0;
                return parse_multi_bulk_impl(get_current_data(), cost_len);
            }

            int32 parse_multi_bulk_impl(data_ptr_t parent, uint64 &cost_len){
                if (parent->get_value() == "-1")
                {
                    cost_len = 0;
                    return decode_result::success;
                }

                uint64 arr_size = std::stoull(parent->get_value());

                if (!parent->get_clients())
                    parent->get_clients() = std::make_shared<array_data_t>();

                for (uint64 i = parent->get_clients()->size(); i < arr_size; i++){
                    
                    auto pos = get_buf().find("\r\n", get_buf_pos());
                    if (pos == string::npos)
                    {
                        set_current_data(parent);
                        return decode_result::not_enough_total;
                    }

                    int8 ch = get_buf()[get_buf_pos()];

                    data_ptr_t data = reply_data_pool.take();
                    data->clear();
                    data->set_parent(parent);

                    if (ch == '+')
                    {
                        data->set_type(e_type_ok);
                        data->get_value().assign(get_buf().data() + get_buf_pos() + 1, pos - get_buf_pos() - 1);
                        cost_len = 1 + data->get_value().size() + 2;
                        parent->get_clients()->push_back(data);
                    }
                    else if (ch == '-')
                    {
                        data->set_type(e_type_error);
                        data->get_value().assign(get_buf().data() + get_buf_pos() + 1, pos - get_buf_pos() - 1);
                        cost_len = 1 + data->get_value().size() + 2;
                        parent->get_clients()->push_back(data);
                    }
                    else if (ch == ':')
                    {
                        data->set_type(e_type_integer);
                        data->get_value().assign(get_buf().data() + get_buf_pos() + 1, pos - get_buf_pos() - 1);
                        cost_len = 1 + data->get_value().size() + 2;
                        parent->get_clients()->push_back(data);
                    }
                    else if (ch == '$')
                    {
                        data->set_type(e_type_bulk);
                        data->get_value().assign(get_buf().data() + get_buf_pos() + 1, pos - get_buf_pos() - 1);
                        cost_len = 1 + data->get_value().size() + 2;

                        set_buf_pos(get_buf_pos() + cost_len);

                        cost_len = 0;
                        int32 rc = parse_bulk_impl(data, cost_len);
                        if (rc == decode_result::success)
                        {
                            set_buf_pos(get_buf_pos() + cost_len);
                            parent->get_clients()->push_back(data);
                        }
                        else
                        {
                            return decode_result::not_enough_total;
                        }
                    }
                    else if (ch == '*')
                    {
                        data->set_type(e_type_multi_bulk);
                        data->get_value().assign(get_buf().data() + get_buf_pos() + 1, pos - get_buf_pos() - 1);
                        cost_len = 1 + data->get_value().size() + 2;

                        set_buf_pos(get_buf_pos() + cost_len);
                        uint64 sub_cost_len = 0;
                        int32 rc = parse_multi_bulk_impl(data, sub_cost_len);
                        if (rc == decode_result::success)
                        {
                            parent->get_clients()->push_back(data);
                        }
                        else
                        {
//                             delete data;
//                             set_current_data(parent);
                            return decode_result::not_enough_total;
                        }
                    }
                    else
                    {
                        return decode_result::parse_error;
                    }
                }

                return decode_result::success;
            }

        private:
            SHINE_GEN_MEMBER_GETSET(socket_t, socket_fd, = invalid_socket);
            SHINE_GEN_MEMBER_GETSET(string, addr);
            SHINE_GEN_MEMBER_GETSET(string, auth);
            SHINE_GEN_MEMBER_GETSET(int32, mode, = e_mode_sync);
            SHINE_GEN_MEMBER_GETSET(uint32, recv_timeout, = 0);
            SHINE_GEN_MEMBER_GETSET(uint8, decode_step, = e_decode_header);
            SHINE_GEN_MEMBER_GETSET(shine::string, buf);
            SHINE_GEN_MEMBER_GETSET(shine::size_t, buf_pos, = 0);
            SHINE_GEN_MEMBER_GETSET(request, request);
            SHINE_GEN_MEMBER_GETSET(response, response);
            SHINE_GEN_MEMBER_GETSET(data_ptr_t, current_data);
        };

        class async_client : public client_base
        {
        public:
            typedef net::connection* connection_t;

        public:
            async_client()
            {
            }
            ~async_client(){

            }

            void bind_connection(net::connection *conn)
            {
                set_connection(conn);
                conn->set_recv_timeout(get_recv_timeout());

                conn->register_recv_callback(std::bind(&async_client::recv_callback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
                conn->register_send_callback(std::bind(&async_client::send_callback, this, std::placeholders::_1, std::placeholders::_2));
                conn->register_recv_timeout_callback(std::bind(&async_client::recv_timeout_callback, this, std::placeholders::_1));
                conn->register_send_timeout_callback(std::bind(&async_client::send_timeout_callback, this, std::placeholders::_1));
                conn->register_close_callback(std::bind(&async_client::close_callback, this, std::placeholders::_1));

//                 conn->async_recv();
            }

            bool async_call(){
                return true;
            }

        private:
            bool recv_callback(const int8 *data, shine::size_t len, net::connection *conn)
            {
                return client_base::recv_callback(data, len);
            }

            bool send_callback(shine::size_t len, net::connection *conn){
                return true;
            }

            bool recv_timeout_callback(net::connection *conn){
                return false;
            }

            bool send_timeout_callback(net::connection *conn){
                return true;
            }

            void close_callback(net::connection *conn)
            {
                delete this;
            }

        private:
            SHINE_GEN_MEMBER_GETSET(connection_t, connection);
        };

        class sync_client : public client_base
        {
        public:
            sync_client()
            {
                set_mode(e_mode_sync);
            }
            ~sync_client(){
                close_connection();
            }

            void close_connection()
            {
				get_buf().clear();
				get_buf_pos() = 0;
                if (get_socket_fd() != invalid_socket)
                {
                    net::socket::close(get_socket_fd());
                    set_socket_fd(invalid_socket);
                }
            }

            bool reset_socket(){
                if (get_socket_fd() == invalid_socket)
                {
                    set_socket_fd(net::socket::create(AF_INET, SOCK_STREAM, 0));
                    if (get_socket_fd() == invalid_socket)
                        return false;

                    if (get_addr().find(":") == string::npos)
                        return false;

                    if (net::socket::connect(get_socket_fd(), get_addr(), 5000) != net::socket::e_success)
                    {
						std::cout << __FUNCTION__ << " connect failed." << std::endl;
                        close_connection();
                        return false;
                    }
                }

                if (get_recv_timeout() > 0)
                {
                    auto timeout = get_recv_timeout();

                    if (timeout > 0)
                    {
#if (defined SHINE_OS_WINDOWS)
                        setsockopt(get_socket_fd(), SOL_SOCKET, SO_SNDTIMEO, (const char *)&timeout, sizeof(timeout));
                        setsockopt(get_socket_fd(), SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout));
#else
                        timeval tv;
                        tv.tv_sec = timeout / 1000;
                        tv.tv_usec = timeout % 1000;

                        setsockopt(get_socket_fd(), SOL_SOCKET, SO_SNDTIMEO, (char *)&tv, sizeof(struct timeval));
                        setsockopt(get_socket_fd(), SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(struct timeval));
#endif
                    }
                }

                if (!get_auth().empty())
                {
                    string auth_request = request::encode({ "AUTH", get_auth() });
                    ::send(get_socket_fd(), auth_request.data(), (int)auth_request.size(), 0);
                    string auth_response;
                    auth_response.resize(128);
                    ::recv(get_socket_fd(), (int8*)auth_response.data(), (int)auth_response.size(), 0);
                }

                return true;

            }

            bool call(const string &data){

                if (!reset_socket())
                    return false;

                int rc = 0;

                size_t len = data.size();
                while (len > 0)
                {
                    rc = ::send(get_socket_fd(), data.data(), (int)data.size(), 0);

                    if (rc <= 0)
                    {
//                         std::cout << net::socket::get_error() << std::endl();
                        close_connection();
                        return false;
                    }

                    len -= rc;
                }

                shine::string recv_buf;
                recv_buf.resize(2048);

                get_buf().reserve();
                set_buf_pos(0);

                set_decode_step(e_decode_header);

                for (;;)
                {
                    rc = ::recv(get_socket_fd(), (int8*)recv_buf.data(), (int)recv_buf.size(), 0);
                    if (rc <= 0)
                    {
                        std::cout << net::socket::get_error() << std::endl;
                        close_connection();
                        return false;
                    }

                    if (!recv_callback(recv_buf.data(), rc))
                    {
                        close_connection();
                        return false;
                    }

                    if (get_decode_step() == e_decode_done)
                    {
                        set_decode_step(e_decode_header);
                        return true;
                    }
                }

                close_connection();
                return false;
            }

        private:
        };

    }
}
