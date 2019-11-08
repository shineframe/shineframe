
#pragma once

#include <iostream>
#include <chrono>
#include <ctime>
#include <string>
#include <mutex>
#include <pthread.h>
#include <signal.h>
#include "../common/define.hpp"
#include "../util/string.hpp"
#include "../util/timer.hpp"
#include "socket.hpp"


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
        class connection : public peer
        {
        public:
            connection(){
                set_monitor_events(0);
                set_type(peer::e_connection);

                register_recv_callback([](const int8 *data, shine::size_t len, connection *conn)->bool{
                    return true;
                });

                register_send_callback([](shine::size_t len, connection *conn)->bool{
                    return true;
                });

                register_recv_timeout_callback([](connection *conn)->bool{
                    return false;
                });

                register_send_timeout_callback([](connection *conn)->bool{
                    return false;
                });

                register_close_callback([](connection *conn){

                });

            }

            virtual ~connection(){}

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

            void do_recv(){
                context &ctx = get_recv_context();
                int len = ::recv(get_socket_fd(), (void*)ctx.get_buf().data(), ctx.get_buf().size(), 0);

                while (len == (int)ctx.get_buf().size())
                {
                    if (!get_recv_callback()(ctx.get_buf().data(), len, this)){
                        close();
                        return;
                    }

                    len = ::recv(get_socket_fd(), (void*)ctx.get_buf().data(), ctx.get_buf().size(), 0);
                }

                if (len > 0)
                {
                    if (!get_recv_callback()(ctx.get_buf().data(), (shine::size_t)len, this)){
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

            void do_send(){
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
                            return ;
                        }
                    }
                    else if (send_len == 0)
                    {
                        close();
                        return ;
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
                    if (!get_send_callback()(send_len_total, this)){
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

            void start_timer(uint32 timeout, uint64 &id, timeout_callback_t &cb){
                stop_timer(id);

                if (timeout == 0)
                    return;

                get_timer_manager()->cancel_timer(id);
                id = get_timer_manager()->set_timer(timeout, [this, &id, &cb]()->bool{
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

            void start_recv_timeout_timer(){
                start_timer(get_recv_timeout(), get_recv_timeout_timer_id(), get_recv_timeout_callback());
            }

            void stop_recv_timeout_timer(){
                stop_timer(get_recv_timeout_timer_id());
            }

            void start_send_timeout_timer(){
                start_timer(get_send_timeout(), get_send_timeout_timer_id(), get_send_timeout_callback());
            }

            void stop_send_timeout_timer(){
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

                epoll_ctl(get_kernel_fd(), get_bind_epoll() ? EPOLL_CTL_MOD : EPOLL_CTL_ADD, get_socket_fd(), &ee);

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
        public:
            virtual void close(bool force = true){

                if (get_type() == peer::e_connection)
                    get_close_callback()(this);

                if (get_socket_fd() != invalid_socket){
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

        typedef std::function<bool(bool, connection *conn)> connection_callback_t;
        typedef std::function<bool(bool, connection *conn)> accept_callback_t;
        typedef std::function<void(bool, connector *conn)> connect_callback_t;

        class connector : public connection{

        public:
            connector() : connection()
            {
                set_type(peer::e_connector);
            }
            virtual ~connector(){}

            virtual void close(bool force = true){

                connection::close(!get_reconnect());
                if (get_reconnect())
                {
                    if (get_socket_fd() == invalid_socket)
                    {
                        get_timer_manager()->set_timer(get_reconnect_delay(), [this]()->bool{
							if (!this->async_connect()){
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

                set_socket_fd(socket::create(AF_INET, SOCK_STREAM, IPPROTO_TCP));
                if (get_socket_fd() == invalid_socket)
                    return false;

                if (!socket::bind(get_socket_fd(), get_bind_addr().get_address_string()))
                    return false;

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
                else{
                    return false;
                }

                return true;
            }

            void start_connect_timeout_timer(){
            }

            void stop_connect_timeout_timer(){
            }

        public:
            SHINE_GEN_MEMBER_GETSET(bool, reconnect, = true);
            SHINE_GEN_MEMBER_GETSET(uint32, reconnect_delay, = 5000);
            SHINE_GEN_MEMBER_GETREG(connect_callback_t, connect_callback, = nullptr);
            SHINE_GEN_MEMBER_GETSET(uint32, connect_timer_id, = invalid_timer_id);
            SHINE_GEN_MEMBER_GETSET(address_info_t, bind_addr);
        protected:

        };

        class acceptor : public connection{
            friend class proactor_engine;
        public:
            acceptor(){
                set_type(peer::e_acceptor);
            }
            virtual ~acceptor(){}

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

            void on_accept(bool status){
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
                        else{
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

        class proactor_engine{
        public:
            proactor_engine() {
                _epoll_fd = epoll_create(64);
            }

            ~proactor_engine(){
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
                conn->set_bind_addr(bind_info);
                conn->set_name(name);
                conn->set_reconnect(reconnect);
                conn->set_reconnect_delay(reconnect_delay);
                conn->register_connect_callback(cb);

                if (!conn->async_connect()) {
					conn->dump_error("async_connect error.");
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