
#pragma once

#include <iostream>
#include <chrono>
#include <ctime>
#include <string>
#include <mutex>
#include <pthread.h>
#include <signal.h>
#include <unordered_map>
#include "../common/define.hpp"
#include "../util/string.hpp"
#include "../util/timer.hpp"
#include "socket.hpp"
#include <thread>

using namespace std;

#ifndef SHINE_HANDLE_PIPE
#define SHINE_HANDLE_PIPE
//void shine_handle_pipe(int sig)
// {
// }

#endif

namespace shine
{
	namespace net
	{
		class connection;
		typedef std::unordered_map<std::string, connection*> map_connection_t;
		typedef map_connection_t * map_connection_p_t;

		class connection : public peer
		{
		public:
			connection() {
				set_monitor_events(0);
				set_type(peer::e_connection);

				register_recv_callback([](const int8 *data, shine::size_t len, connection *conn)->bool {
					return true;
				});

				register_send_callback([](shine::size_t len, connection *conn)->bool {
					return true;
				});

				register_recv_timeout_callback([](connection *conn)->bool {
					return false;
				});

				register_send_timeout_callback([](connection *conn)->bool {
					return false;
				});

				register_close_callback([](connection *conn) {

				});

			}

			virtual ~connection() {}

			virtual void async_recv()
			{
				if ((get_monitor_events() & EPOLLIN) != EPOLLIN)
				{
					context &ctx = get_recv_context();
					if (ctx.get_buf().size() != ctx.get_recv_some()) {
						ctx.get_buf().resize(ctx.get_recv_some());
					}

					do_monitor_events(EPOLLIN | get_monitor_events());
				}

				start_recv_timeout_timer();
			}

			/**
			*@brief �첽��������
			*@param data ����ָ��
			*@param len ���ݳ���
			*@param flush ��������
			*@return void
			*@warning
			*@note
			*/
			virtual void async_send(const int8 *data, size_t len, bool flush = true)
			{
				iovec_t iov;
				iov.data = (int8 *)data;
				iov.size = len;

				async_sendv(&iov, 1, flush);
			}

			/**
			*@brief �첽��������
			*@param iov ���ݿ�ָ��
			*@param count ���ݿ����
			*@param flush ��������
			*@return void
			*@warning
			*@note
			*/
			virtual void async_sendv(const iovec_t *iov, size_t count, bool flush = true)
			{
				context &ctx = get_send_context();

				for (size_t i = 0; i < count; i++)
					ctx.get_buf().append(iov[i].data, iov[i].size);

				if (!flush)
					return;

				if (get_monitor_events() & EPOLLOUT)
					return;

				do_send();
			}

			virtual void do_recv() {
				context &ctx = get_recv_context();
				int len = ::recv(get_socket_fd(), (void*)ctx.get_buf().data(), ctx.get_buf().size(), 0);

				while (len == (int)ctx.get_buf().size())
				{
					if (!get_recv_callback()(ctx.get_buf().data(), len, this)) {
						close();
						return;
					}

					len = ::recv(get_socket_fd(), (void*)ctx.get_buf().data(), ctx.get_buf().size(), 0);
				}

				if (len > 0)
				{
					if (!get_recv_callback()(ctx.get_buf().data(), (shine::size_t)len, this)) {
						close();
						return;
					}

					start_recv_timeout_timer();

				}
				else if (len < 0)
				{
					int32 err = socket::get_error();
					if (err != EWOULDBLOCK && err != EAGAIN && err != EINTR)
					{
						close();
					}

					start_recv_timeout_timer();
				}
				else if (len == 0)
				{
					close();
				}
			}

			virtual void do_send() {
				context &ctx = get_send_context();
				const char* data = ctx.get_buf().data();
				int len = ctx.get_buf().size();

				if (len == 0)
					return;

				int send_len_total = 0;

				while (send_len_total < len)
				{
					int send_len = ::send(get_socket_fd(), data, len - send_len_total, 0);
					if (send_len < 0)
					{
						int err = socket::get_error();
						if (err == EWOULDBLOCK || err == EAGAIN)
						{
							break;
						}
						else if (err == ECONNRESET)
						{
							close();
							return;
						}
						else if (err == EINTR)
						{
							continue;
						}
						else
						{
							return;
						}
					}
					else if (send_len == 0)
					{
						close();
						return;
					}
					else
					{
						data += send_len;
						send_len_total += send_len;
					}
				}

				//             RETURN:
				ctx.flush(send_len_total);

				if (send_len_total > 0)
				{
					if (!get_send_callback()(send_len_total, this)) {
						close();
					}
				}

				if (ctx.get_buf().size() > 0)
				{
					if ((get_monitor_events() & EPOLLOUT) != EPOLLOUT)
						do_monitor_events(get_monitor_events() | EPOLLOUT);
				}
				else
				{
					if ((get_monitor_events() & EPOLLOUT) == EPOLLOUT)
						do_monitor_events(get_monitor_events() & ~EPOLLOUT);
				}
			}

			void start_timer(uint32 timeout, uint64 &id, timeout_callback_t &cb) {
				stop_timer(id);

				if (timeout == 0)
					return;

				get_timer_manager()->cancel_timer(id);
				id = get_timer_manager()->set_timer(timeout, [this, &id, &cb]()->bool {
					if (!cb(this))
					{
						close();
						id = invalid_timer_id;
						return false;
					}
					else
					{
						return true;
					}
				});
			}

			void stop_timer(uint64 &id)
			{
				get_timer_manager()->cancel_timer(id);
				id = invalid_timer_id;
			}

			void start_recv_timeout_timer() {
				start_timer(get_recv_timeout(), get_recv_timeout_timer_id(), get_recv_timeout_callback());
			}

			void stop_recv_timeout_timer() {
				stop_timer(get_recv_timeout_timer_id());
			}

			void start_send_timeout_timer() {
				start_timer(get_send_timeout(), get_send_timeout_timer_id(), get_send_timeout_callback());
			}

			void stop_send_timeout_timer() {
				stop_timer(get_send_timeout_timer_id());
			}
		public:
			void do_monitor_events(uint32 events)
			{
				struct epoll_event ee;
				ee.data.ptr = this;
				events = events | EPOLLET | EPOLLHUP;
				ee.events = events;

				if (!get_bind_epoll())
					socket::set_noblock(get_socket_fd());

				int rc = epoll_ctl(get_kernel_fd(), get_bind_epoll() ? EPOLL_CTL_MOD : EPOLL_CTL_ADD, get_socket_fd(), &ee);

				if (!get_bind_epoll())
					set_bind_epoll(true);

				set_monitor_events(events);
			}

			void dump_error(const char* title) {
				std::cout << title << " fd:" << get_socket_fd() << " remote:" << get_remote_addr().get_address_string() << " local:" << get_local_addr().get_address_string() << " error:" << socket::get_error() << " " << socket::get_error_str(socket::get_error()) << std::endl;
			}

			SHINE_GEN_MEMBER_GETSET(uint32, recv_timeout, =0);
			SHINE_GEN_MEMBER_GETSET(uint32, send_timeout, = 0);
			SHINE_GEN_MEMBER_GETSET(uint64, recv_timeout_timer_id, = invalid_timer_id);
			SHINE_GEN_MEMBER_GETSET(uint64, send_timeout_timer_id, = invalid_timer_id);
			SHINE_GEN_MEMBER_GET_ONLY(context, recv_context);
			SHINE_GEN_MEMBER_GET_ONLY(context, send_context);
			SHINE_GEN_MEMBER_GETREG(close_callback_t, close_callback, = nullptr);
			SHINE_GEN_MEMBER_GETREG(timeout_callback_t, recv_timeout_callback, = nullptr);
			SHINE_GEN_MEMBER_GETREG(timeout_callback_t, send_timeout_callback, = nullptr);
			SHINE_GEN_MEMBER_GETREG(recv_callback_t, recv_callback, = nullptr);
			SHINE_GEN_MEMBER_GETREG(send_callback_t, send_callback, = nullptr);
			SHINE_GEN_MEMBER_GETSET(address_info_t, local_addr);
			SHINE_GEN_MEMBER_GETSET(address_info_t, remote_addr);

			SHINE_GEN_MEMBER_GETSET(int32, type);

			SHINE_GEN_MEMBER_GETSET(uint32, monitor_events, = 0);
			SHINE_GEN_MEMBER_GETSET(bool, bind_epoll, = false);
			SHINE_GEN_MEMBER_GETSET(map_connection_p_t, map_connection_p, = nullptr);
		public:
			virtual void close(bool force = true) {

				if (get_type() == peer::e_connection || get_type() == peer::e_udp_connector)
					get_close_callback()(this);

				if (get_socket_fd() != invalid_socket) {
					stop_recv_timeout_timer();
					stop_send_timeout_timer();

					struct epoll_event ee;
					ee.data.ptr = this;
					ee.events = 0;

					epoll_ctl(get_kernel_fd(), EPOLL_CTL_DEL, get_socket_fd(), &ee);
					socket::close(get_socket_fd());
					set_socket_fd(invalid_socket);
					set_bind_epoll(false);

				}

				if (force)
					delete this;
			}

		};

		class udp_connection : public connection {
		public:
			udp_connection() {
				set_type(peer::e_udp_connection);
			}
			virtual ~udp_connection() {
				if (_map_connection_p != nullptr)
				{
					_map_connection_p->erase(get_id());
				}
			}

			virtual void close(bool force = true) {

				if (get_close_callback())
					get_close_callback()(this);

				if (force)
					delete this;
			}


			/**
			*@brief 异步接收数据
			*@return void
			*@warning
			*@note
			*/
			virtual void async_recv()
			{
				start_recv_timeout_timer();
			}

			/**
			*@brief 异步发送数据
			*@param iov 数据块指针
			*@param count 数据块个数
			*@param flush 立即发送
			*@return void
			*@warning
			*@note
			*/
            virtual void async_sendv(const iovec_t *iov, size_t count, bool flush = true)
            {
                context &ctx = get_send_context();

                for (size_t i = 0; i < count; i++)
                {
                    const char *data = (const char *)(iov[i].data);
                    size_t size = iov[i].size;
                    size_t sended = 0;
                    if (size == 0)
                    {
                        continue;
                    }
                    for (;;)
                    {
                        int rc = ::sendto(get_socket_fd(), data + sended, size - sended, 0, (const sockaddr *)get_id().data(), get_id().size());
                        if (rc > 0)
                        {
                            sended += rc;
                            if (sended >= size)
                            {
                                break;
                            }
                        }
                        else if (rc <= 0)
                        {
                            dump_error("sendto error.");
                            break;
                        }
                    }
                }
            }

            SHINE_GEN_MEMBER_GETSET(std::string, id);

		};


		typedef std::function<bool(bool, connection *conn)> connection_callback_t;
		typedef std::function<bool(bool, connection *conn)> accept_callback_t;
		typedef std::function<void(bool, connector *conn)> connect_callback_t;

		class connector : public connection {

		public:
			connector() : connection()
			{
				set_type(peer::e_connector);
			}
			virtual ~connector() {}

			virtual void close(bool force = true) {

				force = !get_reconnect();

				if (force)
				{
					connection::close(false);
					delete this;
				}
				else
				{
					connection::close(false);
					if (get_socket_fd() == invalid_socket)
					{
						get_timer_manager()->set_timer(get_reconnect_delay(), [this]()->bool {
							if (!this->async_connect()) {
								dump_error("async_connect error.");
								return true;
							}

							return false;
						});
					}
				}


			}

			virtual bool async_connect()
			{
				socket::close(get_socket_fd());
				set_socket_fd(invalid_socket);
				set_type(peer::e_connector);
				set_monitor_events(0);

				set_socket_fd(socket::create(get_v6() ? AF_INET6 : AF_INET, SOCK_STREAM, IPPROTO_TCP));
				if (get_socket_fd() == invalid_socket)
					return false;

				//                 if (!socket::bind(get_socket_fd(), get_bind_addr().get_address_string()))
				//                     return false;

				auto rc = socket::connect(get_socket_fd(), get_remote_addr().get_address_string(), 0);
				if (rc == socket::e_success)
				{
					set_type(peer::e_connection);
					get_connect_callback()(true, this);
				}
				else if (rc == socket::e_inprocess)
				{
					do_monitor_events(EPOLLOUT);
				}
				else {
					return false;
				}

				return true;
			}

			void start_connect_timeout_timer() {
			}

			void stop_connect_timeout_timer() {
			}

		public:
			SHINE_GEN_MEMBER_GETSET(bool, reconnect, = true);
			SHINE_GEN_MEMBER_GETSET(uint32, reconnect_delay, = 5000);
			SHINE_GEN_MEMBER_GETREG(connect_callback_t, connect_callback, = nullptr);
			SHINE_GEN_MEMBER_GETSET(uint32, connect_timer_id, = invalid_timer_id);
			SHINE_GEN_MEMBER_GETSET(address_info_t, bind_addr);
		protected:

		};

		class udp_connector : public connector {

		public:
			udp_connector() : connector() {
				set_type(peer::e_udp_connector);
				get_recv_context().get_buf().resize(65535);
			}
			virtual ~udp_connector() {}

			// virtual void close(bool force = true)
			// {
			// 	force = !get_reconnect();

			// 	if (get_close_callback())
			// 		get_close_callback()(this);

			// 	stop_recv_timeout_timer();
			// 	stop_send_timeout_timer();

			// 	// 				if (get_close_callback())
			// 	// 					get_close_callback()(this);
			// 	// 
			// 	// 				if (force)
			// 	// 					delete this;

			// 	if (force)
			// 	{
			// 		socket::close(get_socket_fd());
			// 		delete this;
			// 	}
			// 	else
			// 	{
			// 		get_timer_manager()->set_timer(get_reconnect_delay(), [this]()->bool {
			// 			if (!this->async_connect())
			// 				dump_error("async_connect error.");
			// 			return false;
			// 		});
			// 	}
			// }

			virtual bool async_connect()
			{
                if (get_socket_fd() != invalid_socket) {
                    /* code */
                    struct epoll_event ee;
					ee.data.ptr = this;
					ee.events = 0;

					epoll_ctl(get_kernel_fd(), EPOLL_CTL_DEL, get_socket_fd(), &ee);
					socket::close(get_socket_fd());
					set_socket_fd(invalid_socket);
					set_bind_epoll(false);

                }

				set_socket_fd(socket::create(get_v6() ? AF_INET6 : AF_INET, SOCK_DGRAM, 0));
				if (get_socket_fd() == invalid_socket)
					return false;

				//                 if (!socket::bind(get_socket_fd(), get_bind_addr().get_address_string()))
				//                     return false;

				auto rc = socket::connect(get_socket_fd(), get_remote_addr().get_address_string(), 0);
				if (rc == socket::e_success)
				{
                    do_monitor_events(EPOLLIN);
					get_connect_callback()(true, this);
				}
				else {
					return false;
				}

				return true;
			}

			virtual void do_recv() {
				context &ctx = get_recv_context();
				int len = ::recv(get_socket_fd(), (void*)ctx.get_buf().data(), ctx.get_buf().size(), 0);

				while (len > 0)
				{
					if (!get_recv_callback()(ctx.get_buf().data(), len, this)) {
						close();
						return;
					}

					len = ::recv(get_socket_fd(), (void*)ctx.get_buf().data(), ctx.get_buf().size(), 0);
				}

				if (len < 0)
				{
					int32 err = socket::get_error();
					if (err != EWOULDBLOCK && err != EAGAIN && err != EINTR)
					{
						close();
					}

					start_recv_timeout_timer();
				}
				else if (len == 0)
				{
					close();
				}
			}


		public:

		protected:
			SHINE_GEN_MEMBER_GETSET(string, id);
 			SHINE_GEN_MEMBER_GETSET(string, feature);           
			char _addr_buf[128];
			socklen_t _addr_len = 0;
		};

		class acceptor : public connection {
			friend class proactor_engine;
		public:
			acceptor() {
				set_type(peer::e_acceptor);
			}
			virtual ~acceptor() {}

		private:

			virtual bool async_listen_accept(const string &addr)
			{
				if (!socket::parse_addr(addr, _local_addr))
					return false;

				socket::close(get_socket_fd());
				set_socket_fd(invalid_socket);

				socket_t fd = socket::create(AF_INET, SOCK_STREAM, IPPROTO_TCP);
				if (fd == invalid_socket)
					return false;

				set_socket_fd(fd);

				if (!socket::bind(get_socket_fd(), addr))
					return false;

				if (!socket::listen(get_socket_fd(), 1024))
					return false;

				do_monitor_events(EPOLLIN);

				return true;
			}

			void on_accept(bool status) {
				connection *conn = nullptr;
				if (status)
				{
					for (;;) {
						socket_t new_fd = invalid_socket;
						address_info_t new_addr;
						if (socket::accept(get_socket_fd(), new_fd, new_addr))
						{
							conn = new connection();
							conn->set_kernel_fd(get_kernel_fd());
							conn->set_socket_fd(new_fd);
							conn->set_timer_manager(get_timer_manager());
							conn->set_remote_addr(new_addr);
							socket::get_local_addr(new_fd, conn->get_local_addr());

							if (!get_accept_callback()(status, conn))
								close();
						}
						else {
							int err = socket::get_error();
							if (err != EWOULDBLOCK && err != EAGAIN && err != EINTR)
								close();
							return;
						}
					}
				}
				else
				{
					if (!get_accept_callback()(status, conn))
						close();
				}
			}

			SHINE_GEN_MEMBER_GETREG(accept_callback_t, accept_callback, = nullptr);

		};

		class udp_acceptor : public connection {
			friend class proactor_engine;
		public:
			udp_acceptor() {
				set_type(peer::e_udp_acceptor);
				get_recv_context().get_buf().resize(65535);
			}
			virtual ~udp_acceptor() {}

		private:

			virtual bool async_listen_accept(const string &addr)
			{
				socket_t fd = socket::bind(addr, SOCK_DGRAM, get_local_addr());
				if (fd == invalid_socket)
					return false;

				set_socket_fd(fd);

				do_monitor_events(EPOLLIN);

				return true;
			}

			void on_accept(bool status) {
				udp_connection *conn = nullptr;
				if (status)
				{
					for (;;) {
						_addr_len = sizeof(sockaddr) + 16;

						int rc = ::recvfrom(get_socket_fd(), (char *)get_recv_context().get_buf().data(), get_recv_context().get_buf().size(), 0, (sockaddr*)_addr_buf, &_addr_len);

						if (rc > 0)
						{
							std::string id(_addr_buf, _addr_len);
							auto iter = _map_connection.find(id);
							if (iter == _map_connection.end())
							{
								string addrstr;
								sockaddr_in *addr = (sockaddr_in*)_addr_buf;
								addrstr.format("%s:%d", inet_ntoa(addr->sin_addr), ntohs(addr->sin_port));

								conn = new udp_connection();
								conn->set_id(id);
								conn->set_kernel_fd(get_kernel_fd());
								conn->set_socket_fd(get_socket_fd());
								conn->set_timer_manager(get_timer_manager());
								conn->set_map_connection_p(&_map_connection);

								conn->set_local_addr(get_local_addr());
								socket::parse_addr(addrstr, conn->get_remote_addr());
								// 						socket::set_nodelay(fd);

								_map_connection.emplace(id, conn);

								if (!get_accept_callback()(status, conn))
									close();
								else
								{
								}

							}
							else
							{
								conn = (udp_connection*)iter->second;
							}

							if (!conn->get_recv_callback()(get_recv_context().get_buf().data(), rc, conn))
							{
								conn->close();
							}
							else
							{
								conn->async_recv();
							}
						}
						else {
							int err = socket::get_error();
							if (err != EWOULDBLOCK && err != EAGAIN && err != EINTR)
								close();
							return;
						}
					}
				}
				else
				{
					if (!get_accept_callback()(status, conn))
						close();
				}
			}

			void on_accept_2(bool status) {
				connection *conn = nullptr;
				if (status)
				{
					for (;;) {
						_addr_len = sizeof(sockaddr) + 16;

						int rc = ::recvfrom(get_socket_fd(), (char *)get_recv_context().get_buf().data(), get_recv_context().get_buf().size(), 0, (sockaddr*)_addr_buf, &_addr_len);

						if (rc > 0)
						{
							std::string id(_addr_buf, _addr_len);
							auto iter = _map_connection.find(id);
							if (iter == _map_connection.end())
							{
                                
								string addrstr;
								sockaddr_in *addr = (sockaddr_in*)_addr_buf;
								addrstr.format("%s:%d", inet_ntoa(addr->sin_addr), ntohs(addr->sin_port));
                                address_info_t info;
                                socket_t new_fd = socket::bind(get_local_addr().get_address_string(), SOCK_DGRAM, info, false);
                                int rc = socket::connect(new_fd, addrstr, 0);
								conn = new connection();
							//	conn->set_id(id);
								conn->set_kernel_fd(get_kernel_fd());
								conn->set_socket_fd(new_fd);
								conn->set_timer_manager(get_timer_manager());
								conn->set_map_connection_p(&_map_connection);

								conn->set_local_addr(get_local_addr());
								socket::parse_addr(addrstr, conn->get_remote_addr());
								// 						socket::set_nodelay(fd);
                                conn->async_recv();
								_map_connection.emplace(id, conn);

								if (!get_accept_callback()(status, conn))
									close();
								else
								{
								}

							}
							else
							{
								conn = (connection*)iter->second;
							}

							conn->get_recv_callback()(get_recv_context().get_buf().data(), rc, conn);
							conn->async_recv();
						}
						else {
							int err = socket::get_error();
							if (err != EWOULDBLOCK && err != EAGAIN && err != EINTR)
								close();
							return;
						}
					}
				}
				else
				{
					if (!get_accept_callback()(status, conn))
						close();
				}
			}

		protected:
			SHINE_GEN_MEMBER_GETREG(accept_callback_t, accept_callback, = nullptr);
			SHINE_GEN_MEMBER_GETSET(map_connection_t, map_connection);
			char _addr_buf[128];
			socklen_t _addr_len = 0;

		};

		class proactor_engine {
		public:
			proactor_engine() {
				_epoll_fd = epoll_create(64);
			}

			~proactor_engine() {
				if (_epoll_fd != invalid_socket)
					socket::close(_epoll_fd);
			}
		private:

		public:

			void run() {

				struct sigaction action;
				action.sa_flags = 0;
				action.sa_handler = [](int sig) {};

				sigaction(SIGPIPE, &action, NULL);

				const int max_event_size = 64;
				while (!get_stop_flag())
				{
					uint64 timeout = _timer.do_timer();

					struct epoll_event event_arr[max_event_size];

					int num = epoll_wait(_epoll_fd, event_arr, max_event_size, timeout > 0 ? timeout : 1000);

					for (int i = 0; i < num; i++)
					{
						struct epoll_event &ee = event_arr[i];
						connection *conn = (connection*)ee.data.ptr;

						if (conn->get_type() == peer::e_acceptor)
						{
							acceptor *obj = (acceptor*)conn;
							obj->on_accept(ee.events & EPOLLIN);
						}
						else if (conn->get_type() == peer::e_udp_acceptor)
						{
							udp_acceptor *obj = (udp_acceptor*)conn;
							obj->on_accept(ee.events & EPOLLIN);
						}
						else if (conn->get_type() == peer::e_connector)
						{
							connector *obj = (connector*)conn;
							if (ee.events & (EPOLLERR | EPOLLHUP))
							{
								obj->get_connect_callback()(false, obj);
								obj->close();
							}
							else if ((ee.events & EPOLLOUT) == EPOLLOUT)
							{
								obj->set_type(peer::e_connection);
								obj->set_monitor_events(0);
								socket::get_local_addr(obj->get_socket_fd(), obj->get_local_addr());
								obj->async_sendv(0, 0);
								obj->get_connect_callback()(true, obj);
							}
						}
						else if (conn->get_type() == peer::e_udp_connector)
						{
							if (ee.events & (EPOLLERR | EPOLLHUP))
								conn->close();
							else
							{
								if (ee.events & EPOLLIN)
									conn->do_recv();
								if (ee.events & EPOLLOUT)
									conn->do_send();

							}
						}
						else if (conn->get_type() == peer::e_connection)
						{
							if (ee.events & (EPOLLERR | EPOLLHUP))
								conn->close();
							else
							{
								if (ee.events & EPOLLIN)
									conn->do_recv();
								if (ee.events & EPOLLOUT)
									conn->do_send();

							}
						}
					}
				}
			}

			void stop() {
				set_stop_flag(true);
			}


			/**
			*@brief �½�һ���ͻ�������
			*@param name ����
			*@param conn_addr �Զ˵�ַhost:port / ip:port
			*@param cb ���ӻص�
			*@param bind_addr ���˵�ַhost:port / ip:port
			*@param reconnect �Ƿ��Զ�����
			*@param reconnect_delay �Զ��������
			*@return bool
			*@warning
			*@note
			*/
			bool add_connector(const string &name, const string &conn_addr, connect_callback_t cb, const string bind_addr = "0.0.0.0:0", bool reconnect = true, uint32 reconnect_delay = 5000) {

				if (get_epoll_fd() == invalid_socket)
					return false;

				address_info_t conn_info;
				if (!socket::parse_addr(conn_addr, conn_info))
					return false;

				address_info_t bind_info;
				if (!socket::parse_addr(bind_addr, bind_info))
					return false;

				connector *conn = new connector;
				conn->set_timer_manager(&_timer);
				conn->set_kernel_fd(get_epoll_fd());
				conn->set_remote_addr(conn_info);
				conn->set_v6(conn_info.get_v6());
				conn->set_bind_addr(bind_info);
				conn->set_name(name);
				conn->set_reconnect(reconnect);
				conn->set_reconnect_delay(reconnect_delay);
				conn->register_connect_callback(cb);

				if (!conn->async_connect()) {
					conn->dump_error("async_connect error.");
					conn->set_reconnect(false);
					conn->close();
					return false;
				}

				return true;
			}

			/**
*@brief 新建一个客户端连接
*@param name 名称
*@param conn_addr 对端地址host:port / ip:port
*@param cb 连接回调
*@param bind_addr 本端地址host:port / ip:port
*@param reconnect 是否自动重连
*@param reconnect_delay 自动重连间隔
*@return bool
*@warning
*@note
*/

			bool add_udp_connector(const string &name, const string &conn_addr, connect_callback_t cb, const string bind_addr = "0.0.0.0:0", bool reconnect = true, uint32 reconnect_delay = 5000) {
				if (get_epoll_fd() == invalid_socket)
					return false;

				address_info_t conn_info;
				if (!socket::parse_addr(conn_addr, conn_info))
					return false;

				address_info_t bind_info;
				if (!socket::parse_addr(bind_addr, bind_info))
					return false;

				udp_connector *conn = new udp_connector;
				conn->set_timer_manager(&_timer);
				conn->set_kernel_fd(get_epoll_fd());
				conn->set_remote_addr(conn_info);
				conn->set_v6(conn_info.get_v6());
				conn->set_bind_addr(bind_info);
				conn->set_name(name);
				conn->set_reconnect(reconnect);
				conn->set_reconnect_delay(reconnect_delay);
				conn->register_connect_callback(cb);

				if (!conn->async_connect()) {
					conn->dump_error("async_connect error.");
					conn->set_reconnect(false);
					conn->close();
					return false;
				}

				return true;
			}

			/**
			*@brief ����һ���������������
			*@param name ��������
			*@param addr ������ַip:port
			*@param cb �����ӻص�����
			*@return bool
			*@warning
			*@note
			*/
			bool add_acceptor(const string &name, const string &addr, accept_callback_t cb) {
				if (get_epoll_fd() == invalid_socket)
					return false;

				address_info_t info;
				if (!socket::parse_addr(addr, info))
					return false;

				acceptor *conn = new acceptor;
				conn->set_timer_manager(&_timer);
				conn->set_kernel_fd(get_epoll_fd());
				conn->set_local_addr(info);
				conn->set_name(name);
				conn->register_accept_callback(cb);

				if (!conn->async_listen_accept(addr)) {
					conn->close();
					return false;
				}

				return true;
			}

			/**
			*@brief 新增一个UDP服务端侦听对象
			*@param name 连接名称
			*@param addr 侦听地址ip:port
			*@param cb 新连接回调对象
			*@return bool
			*@warning
			*@note
			*/
			bool add_udp_acceptor(const string &name, const string &addr, accept_callback_t cb) {
				if (get_epoll_fd() == invalid_socket)
					return false;

				udp_acceptor *conn = new udp_acceptor;
				conn->set_timer_manager(&_timer);
				conn->set_kernel_fd(get_epoll_fd());
				conn->set_name(name);
				conn->register_accept_callback(cb);

				if (!conn->async_listen_accept(addr)) {
					conn->close();
					return false;
				}

				return true;
				return true;
			}



			/**
			*@brief ʹ�����е�socket����һ�����Ӷ���
			*@param name ��������
			*@param fd socket�׽���
			*@param cb ���ӻص�����
			*@return bool
			*@warning
			*@note
			*/
			bool add_connection(const string &name, socket_t fd, connection_callback_t cb) {
				if (fd == invalid_socket || get_epoll_fd() == invalid_socket)
					return false;

				connection *conn = new connection;
				conn->set_timer_manager(&_timer);
				conn->set_kernel_fd(get_epoll_fd());
				conn->set_name(name);
				conn->set_socket_fd(fd);
				socket::set_noblock(fd);

				cb(true, conn);
				return true;
			}

		private:
			SHINE_GEN_MEMBER_GETSET(socket_t, epoll_fd, = invalid_socket);
			SHINE_GEN_MEMBER_GETSET(bool, stop_flag, = false);

			SHINE_GEN_MEMBER_GETSET(timer_manager, timer);

		protected:

		};

	}
}