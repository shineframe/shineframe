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
#include "../http/http_client.hpp"
#include "../tls/tls.hpp"

#if (defined SHINE_OS_WINDOWS)
#else
#endif

using namespace std;

namespace shine
{
    namespace https
    {
        class sync_client : public ::shine::http::client_base
        {
        public:
            sync_client(SSL_CTX *ctx_ = nullptr)
            {
				set_sync_mode(true);
				need_free_ctx = ctx_ == nullptr;
				if (need_free_ctx) {
					tls::init();
					ctx = tls::create_context(SSLv23_method(), NULL, NULL);
				}
				else {
					ctx = ctx_;
				}
				ssl = SSL_new(ctx);
            }
            ~sync_client(){
				if (ssl)
					SSL_free(ssl);

				if (need_free_ctx && ctx)
					SSL_CTX_free(ctx);

				close_connection();
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
                        addr += ":443";

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

				SSL_set_fd(ssl, get_socket_fd());

				int ret = 0;
				if ((ret = SSL_connect(ssl)) != 1)
				{
					return false;
				}

				SSL_write(ssl, data.data(), data.size());


                shine::string recv_buf;
                recv_buf.resize(2048);

                set_decode_step(http::e_decode_header);
                get_buf().clear();
                get_response().clear();

                for (;;)
                {
					int r_len = SSL_read(ssl, (void*)recv_buf.data(), recv_buf.size());
					if (r_len > 0) {
						if (!recv_callback(recv_buf.data(), r_len))
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
					else {
						int err = SSL_get_error(ssl, r_len);

						if (err == SSL_ERROR_NONE || err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE) {
							continue;;
						}
						else {
							//error
							return false;
						}
					}
                }

                close_connection();
                return false;
            }

        private:
			SSL *ssl = nullptr;
			SSL_CTX *ctx = nullptr;
			bool need_free_ctx = false;
        };

    }
}
