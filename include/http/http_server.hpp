 /**
 *****************************************************************************
 *
 *@note shineframe开发框架 https://github.com/shineframe/shineframe
 *
 *@file http_server.hpp
 *
 *@brief http服务端
 *
 *@todo 
 *
 *@author sunjian 39215174@qq.com
 *
 *@version 1.0
 *
 *@date 2018/6/14 
 *****************************************************************************
 */
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
#include <regex>
#include "../common/define.hpp"
#include "../net/proactor_engine.hpp"
#include "http_parser.hpp"

#if (defined SHINE_OS_WINDOWS)
#else
#endif

using namespace std;

namespace shine
{
    namespace http
    {
        class server_peer{
        public:
            typedef std::function<bool(const http::request &request, http::response &response)> url_handle_t;
            typedef std::unordered_map<shine::string, url_handle_t, std::hash<std::string>, cmp_string> url_handle_map_t;

        public:
            server_peer()
            {
                register_url_default_handle([](const http::request &request, http::response &response)->bool{
                    response.set_version(request.get_version());
                    response.set_status_code(200);
                    response.get_body().assign("hello shineframe!");

                    return true;
                });
            }
            ~server_peer(){

            }

            void run(net::connection *conn)
            {
                conn->set_recv_timeout(get_recv_timeout());

                conn->register_recv_callback(std::bind(&server_peer::recv_callback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
                conn->register_send_callback(std::bind(&server_peer::send_callback, this, std::placeholders::_1, std::placeholders::_2));
                conn->register_recv_timeout_callback(std::bind(&server_peer::recv_timeout_callback, this, std::placeholders::_1));
                conn->register_send_timeout_callback(std::bind(&server_peer::send_timeout_callback, this, std::placeholders::_1));
                conn->register_close_callback(std::bind(&server_peer::close_callback, this, std::placeholders::_1));

                conn->async_recv();
            }

        public:
            void register_url_handle(const shine::string &pattern, url_handle_t handle)
            {
                get_handle_map().emplace(pattern, handle);
            }

        private:
            bool recv_callback(const int8 *data, shine::size_t len, net::connection *conn)
            {
                get_buf().append(data, len);
                for (;;)
                {
                    if (get_buf().size() == get_buf_pos())
                    {
                        get_buf().clear();
                        get_buf_pos() = 0;
                        return true;
                    }

                    const int8 *real_data = get_buf().data() + get_buf_pos();
                    if (get_decode_step() == http::e_decode_header)
                    {
                        int8 *header_end = strstr((int8*)real_data, "\r\n\r\n");
                        if (header_end == NULL)
                        {
                            if (get_buf().size() - get_buf_pos() >= 100)
                                return false;

                            return true;
                        }

                        shine::size_t cost_len = get_request().decode_header(real_data, header_end - real_data + 4);
                        if (cost_len < 0)
                            return false;

                        get_buf_pos() += cost_len;
                        get_decode_step() = http::e_decode_body;
                    }

                    if (get_decode_step() == http::e_decode_body)
                    {
                        real_data = get_buf().data() + get_buf_pos();
                        if (get_request().get_content_length() > get_buf().size() - get_buf_pos())
                            return true;

                        get_request().get_body().assign(real_data, get_buf().size() - get_buf_pos());
                        get_buf_pos() += get_request().get_content_length();
                        get_decode_step() = http::e_decode_header;

                        //handle request
                        http::response response;
                        auto iter = get_handle_map().find(get_request().get_url());
                        if (iter != std::end(get_handle_map()))
                        {
                            if (!iter->second(get_request(), response))
                                return false;
                        }
                        else if (!get_url_default_handle()(get_request(), response))
                        {
                                    return false;
                        }

                        shine::string response_data;
                        if (response.encode(response_data))
                            conn->async_send(response_data.data(), response_data.size());

                        get_request().clear();
                    }
                }

                return true;
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
            SHINE_GEN_MEMBER_GETSET(uint32, recv_timeout, = 0);
            SHINE_GEN_MEMBER_GETSET(uint8, decode_step, = http::e_decode_header);
            SHINE_GEN_MEMBER_GETSET(shine::string, buf);
            SHINE_GEN_MEMBER_GETSET(shine::size_t, buf_pos, = 0);

            SHINE_GEN_MEMBER_GETSET(http::request, request);
            SHINE_GEN_MEMBER_GETREG(url_handle_t, url_default_handle);
            SHINE_GEN_MEMBER_GETSET(url_handle_map_t, handle_map);
//             SHINE_GEN_MEMBER_GETSET(url_handle_map_t, handle_map2);
        };

    }
}
