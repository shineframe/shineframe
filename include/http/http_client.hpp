 /**
 *****************************************************************************
 *
 *@note shineframe������� https://github.com/shineframe/shineframe
 *
 *@file http_client.hpp
 *
 *@brief http�ͻ���
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
#include "../common/define.hpp"
#include "../net/socket.hpp"
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
        typedef http::request* request_ptr_t;
        typedef http::response* response_ptr_t;
        typedef std::function<void(bool success, const http::request &request, http::response &response)> response_handle_t;

		inline int htoi(const char *s) {
			int i;
			int n = 0;
			if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
			{
				i = 2;
			}
			else
			{
				i = 0;
			}
			for (; (s[i] >= '0' && s[i] <= '9') || (s[i] >= 'a' && s[i] <= 'z') || (s[i] >= 'A' && s[i] <= 'Z'); ++i)
			{
				if (tolower(s[i]) > '9')
				{
					n = 16 * n + (10 + tolower(s[i]) - 'a');
				}
				else
				{
					n = 16 * n + (tolower(s[i]) - '0');
				}
			}
			return n;
		}

        class client_base{
        public:
            client_base()
            {
            }
            virtual ~client_base(){

            }

            bool recv_callback(const int8 *data, shine::size_t len)
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

                    if (get_decode_step() == http::e_decode_header)
                    {
                        auto pos = get_buf().find("\r\n\r\n", get_buf_pos());
                        if (pos == string::npos)
                        {
                            if (get_buf().size() - get_buf_pos() >= 1024)
                                return false;

                            return true;
                        }

                        shine::size_t cost_len = get_response().decode_header(get_buf().data() + get_buf_pos(), (pos - get_buf_pos()) + 4);
                        if (cost_len == (shine::size_t)-1)
                            return false;

                        get_buf_pos() += cost_len;
                        get_decode_step() = http::e_decode_body;
                    }

                    if (get_decode_step() == http::e_decode_body)
                    {
						if (!get_response().get_is_chunk()) {
							if (get_response().get_content_length() > get_buf().size() - get_buf_pos())
								return true;

							get_response().get_body().assign(get_buf().data() + get_buf_pos(), get_buf().size() - get_buf_pos());
							set_buf_pos(get_buf_pos() + get_response().get_content_length());
						}
						else {
							shine::string &buf = get_buf();
							shine::size_t pos = get_buf_pos();
							shine::string &body = get_response().get_body();
							body.clear();

							while (true) {
								auto flag = buf.find("\r\n", pos);
								if (flag != shine::string::npos) {
									shine::string tmp = buf.substr(pos, flag - pos);
									pos = flag + 2;
									std::size_t len = htoi(tmp.c_str());

									if (len > 0) {
										if (buf.size() < pos + len + 2)
										{
											return true;
										}

										body.append(buf.data() + pos, len);
										pos += len + 2;
									}
									else
									{
										break;
									}

								}
								else {
									return true;
								}
							}
						}

                        if (get_sync_mode())
                        {
                            get_decode_step() = http::e_decode_done;
                            return true;
                        }
                        else
                        {
                            get_decode_step() = http::e_decode_header;

                            //handle response
                            if (get_response_handle() != nullptr)
                                get_response_handle()(true, get_request(), get_response());
                        }
                    }
                }

                return true;
            }

        private:
            SHINE_GEN_MEMBER_GETSET(socket_t, socket_fd, = invalid_socket);
            SHINE_GEN_MEMBER_GETSET(bool, sync_mode, = false);
            SHINE_GEN_MEMBER_GETSET(uint32, recv_timeout, = 3000);
            SHINE_GEN_MEMBER_GETSET(uint8, decode_step, = http::e_decode_header);
            SHINE_GEN_MEMBER_GETSET(shine::string, buf);
            SHINE_GEN_MEMBER_GETSET(shine::size_t, buf_pos, = 0);

            SHINE_GEN_MEMBER_GETREG(response_handle_t, response_handle);
            SHINE_GEN_MEMBER_GETSET(http::request, request);
            SHINE_GEN_MEMBER_GETSET(http::response, response);
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

            bool call(response_handle_t callback = nullptr){
                if (callback != nullptr)
                    register_response_handle(callback);

                if (get_response_handle() == nullptr)
                    return false;

                string data;
                if (!get_request().encode(data))
                    return false;

                get_connection()->async_send(data.data(), data.size());
                get_connection()->async_recv();

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
                set_sync_mode(true);
            }
            ~sync_client(){

            }

            void close_connection()
            {
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

                    shine::string addr = get_request().get_host();
                    if (addr.find(":") == string::npos)
                        addr += ":80";

                    if (net::socket::connect(get_socket_fd(), addr, get_recv_timeout()) != net::socket::e_success)
                    {
                        close_connection();
                        return false;
                    }
                }

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

                return true;
            }

            bool call(bool close_on_finish = true){
                string data;
                if (!get_request().encode(data))
                    return false;

                if (!reset_socket())
                    return false;

                int rc = 0;
                size_t len = data.size();
                while (len > 0)
                {
                    rc = ::send(get_socket_fd(), data.data(), (int)data.size(), 0);

                    if (rc <= 0)
                    {
                        close_connection();
                        return false;
                    }

                    len -= rc;
                }

                shine::string recv_buf;
                recv_buf.resize(2048);

                set_decode_step(http::e_decode_header);
                get_buf().clear();
                get_response().clear();

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

                    if (get_decode_step() == http::e_decode_done)
                    {
                        if (close_on_finish)
                            close_connection();
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
