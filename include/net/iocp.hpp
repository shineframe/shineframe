
#pragma once

#include <iostream>
#include <chrono>
#include <ctime>
#include <string>
#include <mutex>
#include "../common/define.hpp"
#include "../util/string.hpp"
#include "../util/timer.hpp"
#include "socket.hpp"

using namespace std;

namespace shine
{
    namespace net
    {
        /**
         @brief TCP连接类
         */
        /**
        *
         @brief 
         *
         * TODO: long description
         *
         */
        
        class connection : public peer
        {
        public:
            connection(){
                get_recv_context().set_parent(this);
                get_send_context().set_parent(this);
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

            /** 
             *@brief 异步接收数据
             *@return void 
             *@warning 
             *@note 
            */
            virtual void async_recv()
            {
                context &ctx = get_recv_context();
                if (ctx.get_status() != context::e_idle)
                    return;

                if (ctx.get_buf().size() != ctx.get_recv_some()) {
                    ctx.get_buf().resize(ctx.get_recv_some());
                }

                DWORD bytes = 0;
                DWORD flags = 0;

                WSABUF &wsabuf = ctx.get_WSABuf();
                wsabuf.buf = (CHAR*)ctx.get_buf().data();
                wsabuf.len = (ULONG)ctx.get_buf().size();

                if (WSARecv(get_socket_fd(), &wsabuf, 1, &bytes, &flags, (LPOVERLAPPED)&ctx, nullptr) == SOCKET_ERROR)
                {
                    if (WSAGetLastError() != WSA_IO_PENDING)
                    {
                        /////
                    }
                }

                ctx.set_status(context::e_recv);
                start_recv_timeout_timer();
            }

            /** 
             *@brief 异步发送数据
             *@param data 数据指针
             *@param len 数据长度
             *@param flush 立即发送
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
                DWORD bytes = 0;
                context &ctx = get_send_context();

                for (size_t i = 0; i < count; i++)
                    ctx.get_buf().append(iov[i].data, iov[i].size);

                if (!flush || ctx.get_buf().size() == 0 || ctx.get_status() != context::e_idle)
                    return;

                WSABUF &wsabuf = ctx.get_WSABuf();
                wsabuf.buf = (CHAR*)ctx.get_buf().data();
                wsabuf.len = (ULONG)ctx.get_buf().size();

                if (WSASend(get_socket_fd(), &wsabuf, 1, &bytes, 0, (LPOVERLAPPED)&get_send_context(), nullptr) == SOCKET_ERROR)
                {
                    if (WSAGetLastError() != WSA_IO_PENDING)
                    {
						dump_error("WSASend error.");
                    }
                }

                ctx.set_status(context::e_send);
            }

            void start_timer(uint32 timeout, uint64 &id, timeout_callback_t &cb){
                stop_timer(id);

                if (timeout == 0)
                    return;

                get_timer_manager()->cancel_timer(id);
                id = get_timer_manager()->set_timer(timeout, [this, &id, &cb]()->bool{
                    if (!cb(this))
                    {
                        id = invalid_timer_id;
                        shutdown(this->get_socket_fd(), SD_SEND);

//                         close();
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

			void dump_error(const char* title) {
				std::cout << title << " fd:" << get_socket_fd() << " remote:" << get_remote_addr().get_address_string() << " local:" << get_local_addr().get_address_string() << " error:" << socket::get_error() << " " << socket::get_error_str(socket::get_error()) << std::endl;
			}

            SHINE_GEN_MEMBER_GETSET(uint32, recv_timeout, = 0);
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
            SHINE_GEN_MEMBER_GETSET(int32, status, = context::e_idle);

        public:
            virtual void close(bool force = true){

                if ((get_recv_context().get_status() != context::e_recv
                    && get_recv_context().get_status() != context::e_connect
                    && get_recv_context().get_status() != context::e_accept)

                    || get_send_context().get_status() != context::e_send)
                {
                    stop_recv_timeout_timer();
                    stop_send_timeout_timer();

                    if (get_close_callback())
                        get_close_callback()(this);
                    
                    socket::close(get_socket_fd());
                    set_socket_fd(invalid_socket);

                    if (force)
                        delete this;
                }
                else
                {
                    set_status(context::e_close);
                }
            }

        };

        typedef std::function<bool(bool, connection *conn)> connection_callback_t;
        typedef std::function<bool(bool, connection *conn)> accept_callback_t;
        typedef std::function<void(bool, connector *conn)> connect_callback_t;

        class connector : public connection{

        public:
            connector() : connection(){
                set_type(peer::e_connector);
            }
            virtual ~connector(){}

            virtual void close(bool force = true)
            {
                connection::close(!get_reconnect());

                if (get_reconnect())
                {
                    if (get_socket_fd() == invalid_socket)
                    {
                        get_timer_manager()->set_timer(get_reconnect_delay(), [this]()->bool{
                            if (!this->async_connect())
                                dump_error("async_connect error.");
                            return false;
                        });
                    }
                }
            }

            virtual bool async_connect()
            {
                socket::close(get_socket_fd());
                set_socket_fd(invalid_socket);

                GUID guid = WSAID_CONNECTEX;
                DWORD ret;
                LPFN_CONNECTEX func = nullptr;

                socket_t fd = socket::create(AF_INET, SOCK_STREAM, IPPROTO_TCP);
                if (fd == invalid_socket)
                    return false;

                set_socket_fd(fd);

                if (CreateIoCompletionPort((HANDLE)get_socket_fd(), get_kernel_fd(), 0, 0) != get_kernel_fd())
                    return false;

                if (WSAIoctl(get_socket_fd(), SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), &func
                    , sizeof(func), &ret, 0, 0) == SOCKET_ERROR)
                    return false;

                if (!socket::bind(get_socket_fd(), get_bind_addr().get_address_string()))
                    return false;

                struct sockaddr_in address;
                memset(&address, 0, sizeof(address));
                address.sin_family = AF_INET;
                address.sin_addr.s_addr = inet_addr(get_remote_addr().get_ip().c_str());
                address.sin_port = htons(get_remote_addr().get_port());

                DWORD bytes = 0;
                get_recv_context().set_status(context::e_connect);
                get_recv_context().set_parent(this);
                get_send_context().set_status(context::e_idle);
                get_send_context().set_parent(this);

                if (!func(fd, (SOCKADDR*)(&address), sizeof(address), nullptr, 0, &bytes, &(get_recv_context())))
                {
                    if (socket::get_error() != ERROR_IO_PENDING)
                        return false;
                }

                return true;
            }


        public:

        public:
            SHINE_GEN_MEMBER_GETREG(connect_callback_t, connect_callback, = nullptr);
            SHINE_GEN_MEMBER_GETSET(bool, reconnect, = true);
            SHINE_GEN_MEMBER_GETSET(uint32, reconnect_delay, = 5000);
            SHINE_GEN_MEMBER_GETSET(uint32, reconnect_timer_id, = invalid_timer_id);
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
                if (!socket::parse_addr(addr, get_local_addr()))
                    return false;

                socket::close(get_socket_fd());
                set_socket_fd(invalid_socket);

                GUID guid = WSAID_ACCEPTEX;
                DWORD ret;
                set_acceptex_func(nullptr);

                socket_t fd = socket::create(AF_INET, SOCK_STREAM, IPPROTO_TCP);
                if (fd == invalid_socket)
                    return false;

                set_socket_fd(fd);

                if (CreateIoCompletionPort((HANDLE)get_socket_fd(), get_kernel_fd(), 0, 0) != get_kernel_fd())
                    return false;

                if (WSAIoctl(get_socket_fd(), SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), &_acceptex_func
                    , sizeof(get_acceptex_func()), &ret, 0, 0) == SOCKET_ERROR)
                    return false;

                if (!socket::bind(get_socket_fd(), addr))
                    return false;

                if (!socket::listen(get_socket_fd(), 1024))
                    return false;

                get_recv_context().set_parent(this);

                return async_accept();
            }

            bool async_accept(){

                if (get_new_conn_fd() != invalid_socket)
                    socket::close(get_new_conn_fd());

                set_new_conn_fd(socket::create(AF_INET, SOCK_STREAM, IPPROTO_TCP));
                if (get_new_conn_fd() == invalid_socket)
                    return false;

                DWORD address_size = sizeof(sockaddr_in)+16;

                _bytes_len = 0;

                get_recv_context().set_status(context::e_accept);

                if (get_acceptex_func()(get_socket_fd(), get_new_conn_fd(), _addr_buf, 0, address_size, address_size, &_bytes_len,
                    (OVERLAPPED*)&get_recv_context()) == FALSE)
                {
                    if (WSAGetLastError() != ERROR_IO_PENDING)
                    {
						dump_error("acceptex error");
                        return false;
                    }
                    else
                        return true;
                }

                on_accept(true);
                return true;
            }

            void on_accept(bool status){
                connection *conn = nullptr;
                if (status)
                {
                    if (setsockopt(get_new_conn_fd(), SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char *)&get_socket_fd(), sizeof(get_socket_fd())) != 0)
                    {
                        socket::close(get_new_conn_fd());
                        set_new_conn_fd(invalid_socket);
                        async_accept();
                        return;
                    }

                    if (CreateIoCompletionPort((HANDLE)get_new_conn_fd(), get_kernel_fd(), 0, 0) != get_kernel_fd())
                    {
                        socket::close(get_new_conn_fd());
                        set_new_conn_fd(invalid_socket);
                        async_accept();
                        return;
                    }

                    conn = new connection();
                    conn->set_kernel_fd(get_kernel_fd());
                    conn->set_socket_fd(get_new_conn_fd());
                    conn->set_timer_manager(get_timer_manager());

                    socket::get_local_addr(conn->get_socket_fd(), conn->get_local_addr());
                    socket::get_remote_addr(conn->get_socket_fd(), conn->get_remote_addr());
                    socket::set_nodelay(get_new_conn_fd());
                    set_new_conn_fd(invalid_socket);

                }

                if (!get_accept_callback()(status, conn))
                    close();
                else
                    async_accept();
            }

        protected:
            SHINE_GEN_MEMBER_GETREG(accept_callback_t, accept_callback, = nullptr);

            SHINE_GEN_MEMBER_GETSET(LPFN_ACCEPTEX, acceptex_func, = nullptr);
            SHINE_GEN_MEMBER_GETSET(socket_t, new_conn_fd, = invalid_socket);
            char _addr_buf[128];
            DWORD _bytes_len = 0;
        };

        class proactor_engine{
        public:
            proactor_engine() {
                socket::init();
                _iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
            }
        private:
            void handle_success(context *ctx, DWORD len){
                if (ctx == nullptr)
                    return;

                if (ctx->get_status() == context::e_connect)
                {
                    connector *&obj = (connector *&)ctx->get_parent();

                    if (obj->get_status() == context::e_close)
                    {
                        ctx->set_status(context::e_close);
                    }
                    else
                    {
                        obj->get_recv_context().set_status(context::e_idle);
                        obj->get_send_context().set_status(context::e_idle);
                        socket::get_local_addr(obj->get_socket_fd(), obj->get_local_addr());
                        obj->async_sendv(0, 0);
                        obj->get_connect_callback()(true, obj);
                    }
                }
                else if (ctx->get_status() == context::e_accept)
                {
                    acceptor *&obj = (acceptor *&)ctx->get_parent();
                    if (obj->get_status() == context::e_close)
                    {
                        ctx->set_status(context::e_close);
                        obj->close();
                    }
                    else
                    {
                         obj->on_accept(true);
                    }
                }
                else if (ctx->get_status() == context::e_recv)
                {
                    connection *&obj = (connection *&)ctx->get_parent();

                    if (obj->get_status() == context::e_close)
                    {
                        ctx->set_status(context::e_close);
                        obj->close();
                        return;
                    }

                    recv_callback_t &cb = obj->get_recv_callback();

                    ctx->set_status(context::e_idle);
                    if (len > 0)
                    {
                        if (!cb(ctx->get_buf().data(), len, obj))
                        {
                            ctx->set_status(context::e_close);
                            obj->close();
                        }
                        else
                        {
                            obj->async_recv();
                        }
                    }
                    else
                    {
                        obj->close();
                    }
                }
                else if (ctx->get_status() == context::e_send)
                {
                    connection *&obj = (connection *&)ctx->get_parent();

                    if (obj->get_status() == context::e_close)
                    {
                        ctx->set_status(context::e_idle);
                        obj->close();
                        return;
                    }

                    send_callback_t &cb = obj->get_send_callback();

                    if (len > 0)
                    {
                        ctx->flush(len);

                        if (!cb(len, obj))
                        {
                            ctx->set_status(context::e_close);
                            obj->close();
                        }
                        else
                        {
                            ctx->set_status(context::e_idle);

                            if (ctx->get_buf().size() > 0)
                            {
                                obj->async_send(nullptr, 0);
                            }
                        }
                    }
                    else
                    {
                        ctx->set_status(context::e_close);
                        obj->close();
                    }
                }

            }

            void handle_failed(context *ctx, DWORD len){
                if (ctx == nullptr)
                    return;
                 if (ctx->get_status() == context::e_connect)
                {
                     connector *&obj = (connector *&)ctx->get_parent();
                     obj->get_connect_callback()(false, obj);

                     if (obj->get_reconnect())
                     {
                         obj->get_timer_manager()->set_timer(obj->get_reconnect_delay(), [&obj]()->bool{
							 if (!obj->async_connect()) {
								 obj->dump_error("async_connect error.");
								 return true;
							 }
							 return false;
                         });
                     }
                     else
                     {
                         ctx->set_status(context::e_close);
                         obj->set_status(context::e_close);
                         obj->close();
                     }
                }
                else if (ctx->get_status() == context::e_accept)
                {
                    acceptor *&obj = (acceptor *&)ctx->get_parent();

                    if (obj->get_status() == context::e_close)
                    {
                        ctx->set_status(context::e_close);
                        obj->close();
                    }
                    else
                    {
                        if (!obj->get_accept_callback()(false, obj))
                            obj->close();
                    }
                }
                else if (ctx->get_status() == context::e_recv)
                {
                    connection *&obj = (connection *&)ctx->get_parent();
                    ctx->set_status(context::e_close);
                    obj->close();
                }
                else if (ctx->get_status() == context::e_send)
                {
                    connection *&obj = (connection *&)ctx->get_parent();
                    ctx->set_status(context::e_close);
                    obj->close();
                }

            }
        public:

            void run() {

                while (!get_stop_flag())
                {
                    DWORD timeout = (DWORD)_timer.do_timer();

                    void *key = nullptr;
                    context *ctx = nullptr;
                    DWORD len = 0;
                    BOOL rc = GetQueuedCompletionStatus(_iocp, &len, (PULONG_PTR)&key, (LPOVERLAPPED*)&ctx,
                        timeout > 0 ? timeout : 1000);
                    
					int error = socket::get_error();
                    if (rc == FALSE)
                    {
                        if (error == WAIT_TIMEOUT)
                            continue;
                    }

                    if (ctx->get_status() == context::e_exit)
                    {
                        delete ctx;
                        break;
                    }

                    if (rc == FALSE)
                        handle_failed(ctx, len);
                    else
                        handle_success(ctx, len);

                }
            }

            void stop() {
				set_stop_flag(true);
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

            bool add_connector(const string &name, const string &conn_addr, connect_callback_t cb, const string bind_addr = "0.0.0.0:0", bool reconnect = true, uint32 reconnect_delay = 5000) {
                if (_iocp == nullptr)
                    return false;

                address_info_t conn_info;
                if (!socket::parse_addr(conn_addr, conn_info))
                    return false;

				auto ip_arr = socket::dns(conn_info.get_ip());
				if (ip_arr.size() > 0)
				{
					conn_info.set_ip(ip_arr[0].first);
				}

                address_info_t bind_info;
                if (!socket::parse_addr(bind_addr, bind_info))
                    return false;

                connector *conn = new connector;
                conn->set_timer_manager(&_timer);
                conn->set_kernel_fd(_iocp);
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
             *@brief 新增一个服务端侦听对象
             *@param name 连接名称
             *@param addr 侦听地址ip:port
             *@param cb 新连接回调对象
             *@return bool 
             *@warning 
             *@note 
            */
            bool add_acceptor(const string &name, const string &addr, accept_callback_t cb) {
                if (_iocp == nullptr)
                    return false;

                address_info_t info;
                if (!socket::parse_addr(addr, info))
                    return false;

                acceptor *conn = new acceptor;

                conn->set_timer_manager(&_timer);
                conn->set_kernel_fd(_iocp);
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
             *@brief 使用已有的socket创建一个连接对象
             *@param name 连接名称
             *@param fd socket套接字
             *@param cb 连接回调对象
             *@return bool 
             *@warning 
             *@note 
            */
            bool add_connection(const string &name, socket_t fd, connection_callback_t cb) {
                if (fd == invalid_socket || _iocp == nullptr)
                    return false;

                if (CreateIoCompletionPort((HANDLE)fd, _iocp, 0, 0) != _iocp)
                    return false;


                connection *conn = new connection;
                conn->set_timer_manager(&_timer);
                conn->set_kernel_fd(_iocp);
                conn->set_name(name);
                conn->set_socket_fd(fd);
                conn->get_recv_context().set_parent(conn);
                conn->get_send_context().set_parent(conn);
                socket::set_noblock(fd);

                cb(true, conn);


                return true;
            }

        private:
            HANDLE _iocp;///<完成端口句柄
			SHINE_GEN_MEMBER_GETSET(timer_manager, timer);
			SHINE_GEN_MEMBER_GETSET(bool, stop_flag);

        protected:

        };

    }
}