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
#include <unordered_set>
#include <vector>
#include <functional>
#include <regex>
#include "../common/define.hpp"
#include "../net/proactor_engine.hpp"
#include "../concurrent_queue/concurrent_queue.hpp"
#include "http_parser.hpp"

#if (defined SHINE_OS_WINDOWS)
#else
#endif

// using namespace std;

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

        protected:
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
                            if (get_buf().size() - get_buf_pos() >= 1024)
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

		protected:
            SHINE_GEN_MEMBER_GETSET(uint32, recv_timeout, = 0);
            SHINE_GEN_MEMBER_GETSET(uint8, decode_step, = http::e_decode_header);
            SHINE_GEN_MEMBER_GETSET(shine::string, buf);
            SHINE_GEN_MEMBER_GETSET(shine::size_t, buf_pos, = 0);

            SHINE_GEN_MEMBER_GETSET(http::request, request);
            SHINE_GEN_MEMBER_GETREG(url_handle_t, url_default_handle);
            SHINE_GEN_MEMBER_GETSET(url_handle_map_t, handle_map);
//             SHINE_GEN_MEMBER_GETSET(url_handle_map_t, handle_map2);
        };

		class server_peer_multi_thread : public server_peer {
		public:
			enum thread_num_limit_t {
				e_min = 1,
				e_max = 256,
			};

			typedef std::function<bool(const http::request &request, http::response &response)> url_handle_t;
			typedef std::unordered_map<shine::string, url_handle_t, std::hash<std::string>, cmp_string> url_handle_map_t;

			struct context_t {
				http::request request;
				http::response response;
				net::connection *conn = nullptr;
				url_handle_t handle = nullptr;
			};

			typedef block_concurrent_queue<context_t*> request_queue_t;
			typedef concurrent_queue<context_t*> response_queue_t;

		public:
			server_peer_multi_thread()
			{
				register_url_default_handle([](const http::request &request, http::response &response)->bool {
					response.set_version(request.get_version());
					response.set_status_code(200);
					response.get_body().assign("hello shineframe!");

					return true;
				});
			}
			~server_peer_multi_thread() {

			}

		private:
			static std::unordered_set<net::connection*> & get_connection_set() {
				static std::unordered_set<net::connection*> set_;
				return set_;
			}

			static std::recursive_mutex& get_mutex() {
				static std::recursive_mutex mutex;
				return mutex;
			}

			static void add_connection(net::connection* conn) {
				std::unique_lock<std::recursive_mutex> lock(get_mutex());
				get_connection_set().insert(conn);
			}

			static void del_connection(net::connection* conn) {
				std::unique_lock<std::recursive_mutex> lock(get_mutex());
				get_connection_set().erase(conn);
			}

			static bool exist_connection(net::connection *conn) {
				std::unique_lock<std::recursive_mutex> lock(get_mutex());
				return get_connection_set().find(conn) != get_connection_set().end();
			}

			static request_queue_t& get_request_queue() {
				static request_queue_t que;
				return que;
			}

			static response_queue_t& get_response_queue() {
				static response_queue_t que;
				return que;
			}

		public:
			static void setup(net::proactor_engine&engine, uint32 thread_num = std::thread::hardware_concurrency()) {
				static std::vector<std::thread> work_threads;
				if (work_threads.size() > 0)
					return;

				if (thread_num < e_min)
					thread_num = e_min;
				else if (thread_num > e_max)
					thread_num = e_max;

				for (uint32 i = 0;i < thread_num; i++)
				{
					work_threads.emplace_back([]() {
						for (;;)
						{
							context_t *task;
							get_request_queue().pop(task);
							
							task->handle(task->request, task->response);
							get_response_queue().push(task);
							
						}
					});
				}

				engine.get_timer().set_timer(1, []()->bool {
					context_t* arr[200];
					std::size_t count = get_response_queue().pop_bulk(arr, 200);
					for (std::size_t i = 0; i < count; i++)
					{
						if (exist_connection(arr[i]->conn))
						{
							shine::string response_data;
							if (arr[i]->response.encode(response_data))
								arr[i]->conn->async_send(response_data.data(), response_data.size());

							delete arr[i];
						}
					}
					return true;
				});
			}


			void run(net::connection *conn)
			{
				add_connection(conn);
				conn->set_recv_timeout(get_recv_timeout());

				conn->register_recv_callback(std::bind(&server_peer_multi_thread::recv_callback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
				conn->register_send_callback(std::bind(&server_peer_multi_thread::send_callback, this, std::placeholders::_1, std::placeholders::_2));
				conn->register_recv_timeout_callback(std::bind(&server_peer_multi_thread::recv_timeout_callback, this, std::placeholders::_1));
				conn->register_send_timeout_callback(std::bind(&server_peer_multi_thread::send_timeout_callback, this, std::placeholders::_1));
				conn->register_close_callback(std::bind(&server_peer_multi_thread::close_callback, this, std::placeholders::_1));

				conn->async_recv();
			}

		public:
			void register_url_handle(const shine::string &pattern, url_handle_t handle)
			{
				get_handle_map().emplace(pattern, handle);
			}

		protected:
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

					if (ctx == nullptr)
						ctx = new context_t;

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

						shine::size_t cost_len = ctx->request.decode_header(real_data, header_end - real_data + 4);
						if (cost_len < 0)
							return false;

						get_buf_pos() += cost_len;
						get_decode_step() = http::e_decode_body;
					}

					if (get_decode_step() == http::e_decode_body)
					{
						real_data = get_buf().data() + get_buf_pos();
						if (ctx->request.get_content_length() > get_buf().size() - get_buf_pos())
							return true;

						ctx->request.get_body().assign(real_data, get_buf().size() - get_buf_pos());
						get_buf_pos() += ctx->request.get_content_length();
						get_decode_step() = http::e_decode_header;

						//handle request
						auto iter = get_handle_map().find(ctx->request.get_url());
						if (iter != std::end(get_handle_map()))
							ctx->handle = iter->second;
						else
							ctx->handle = get_url_default_handle();

						ctx->conn = conn;

						get_request_queue().push(ctx);
						ctx = nullptr;
					}
				}

				return true;
			}

			bool send_callback(shine::size_t len, net::connection *conn) {
				return true;
			}

			bool recv_timeout_callback(net::connection *conn) {
				return false;
			}

			bool send_timeout_callback(net::connection *conn) {
				return true;
			}

			void close_callback(net::connection *conn)
			{
				del_connection(conn);
				delete this;
			}
		protected:
			bool save_conn = false;
			context_t *ctx = nullptr;
		};
    }
}
