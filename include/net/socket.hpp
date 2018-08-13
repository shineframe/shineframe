 /**
 *****************************************************************************
 *
 *@file socket.hpp
 *
 *@brief socket基本接口封装
 *
 *@todo 
 * 
 *@note shineframe开发框架 https://github.com/shineframe/shineframe
 *
 *@author sunjian 39215174@qq.com
 *
 *@version 1.0
 *
 *@date 2018/6/15 
 *****************************************************************************
 */
#pragma once

#include <iostream>
#include <chrono>
#include <ctime>
#include <string>
#include <mutex>
#include <map>
#include <functional>
#include "../common/define.hpp"
#include "../util/string.hpp"
#include "../util/timer.hpp"

#if (defined SHINE_OS_WINDOWS)
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <mswsock.h>
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
typedef SOCKET socket_t;
#define invalid_socket INVALID_SOCKET

#define LOAD_WSA_FUNC(guid, pfunc) {GUID id = guid; socket::load_wsa_func(id, (void *&)pfunc); }
//             LOAD_WSA_FUNC(WSAID_ACCEPTEX, _acceptex);
//             LOAD_WSA_FUNC(WSAID_CONNECTEX, _connectex);
//             LOAD_WSA_FUNC(WSAID_DISCONNECTEX, _disconnectex);
//             LOAD_WSA_FUNC(WSAID_GETACCEPTEXSOCKADDRS, _getacceptexsockaddrs);
//             LOAD_WSA_FUNC(WSAID_TRANSMITFILE, _transmitfile);
//             LOAD_WSA_FUNC(WSAID_TRANSMITPACKETS, _transmitpackets);

#else
#include <cstring>
#include<netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
typedef shine::int32 socket_t;
#define invalid_socket -1
#endif



using namespace std;

namespace shine
{
    namespace net{
        struct address_info_t{
            SHINE_GEN_MEMBER_GETSET(shine::string, ip);
            SHINE_GEN_MEMBER_GETSET(uint16, port);
        };

        class socket{
            friend class proactor_engine;
        public:

            /** 
             *@brief 将地址转成对象
             *@param addr 地址domain:port/host:port
             *@param ret 接收结果的对象
             *@return bool 
             *@warning 
             *@note 
            */
            static bool parse_addr(const string &addr, address_info_t &ret) {
                size_type pos = addr.find(":");
                if (pos == string::npos)
                    return false;

                ret.get_ip().assign(addr.c_str(), pos);
                ret.get_port() = atoi(addr.c_str() + pos + 1);

                return !ret.get_ip().empty();
            }

            static bool get_local_addr(socket_t fd, address_info_t &ret) {
                struct sockaddr_in sa;
                int32 len = sizeof(sa);
                if (!getsockname(fd, (struct sockaddr *)&sa, &len))
                {
                    ret.set_ip(inet_ntoa(sa.sin_addr));
                    ret.set_port(ntohs(sa.sin_port));
                    return true;
                }
                return false;
            }

            static bool get_remote_addr(socket_t fd, address_info_t &ret) {
                struct sockaddr_in sa;
                int32 len = sizeof(sa);
                if (!getpeername(fd, (struct sockaddr *)&sa, &len))
                {
                    ret.set_ip(inet_ntoa(sa.sin_addr));
                    ret.set_port(ntohs(sa.sin_port));

                    return true;
                }
                return false;
            }

            /** 
             *@brief 获取socket错误码
             *@return shine::int32 
             *@warning 
             *@note 
            */
            static int32 get_error() {
#ifdef SHINE_OS_WINDOWS
                return WSAGetLastError();
#else
                return errno;
#endif
            }

            /** 
             *@brief 控制Nagle算法选项
             *@param fd 套接字
             *@param val 是否启用
             *@return void 
             *@warning 
             *@note 
            */
            static void set_nodelay(socket_t fd, bool val = true)
            {
                int opt = val ? 1 : 0;
                ::setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char *)&opt, sizeof(opt));
            }

            /** 
             *@brief 控制阻塞选项
             *@param fd 套接字
             *@param val 是否非阻塞
             *@return void 
             *@warning 
             *@note 
            */
            static void set_noblock(socket_t fd, bool val = true)
            {
#if (defined SHINE_OS_WINDOWS)  
                unsigned long opt = val ? 1 : 0;
                ::ioctlsocket(fd, FIONBIO, &opt);
#else
                int flags = fcntl(fd, F_GETFL, 0);
                if (val)
                    flags |= O_NONBLOCK;
                else
                    flags &= ~O_NONBLOCK;

                fcntl(fd, F_SETFL, flags);
#endif 
            }

            /** 
             *@brief 套接字绑定
             *@param fd 套接字
             *@param addr 绑定地址
             *@return bool 
             *@warning 
             *@note 
            */
            static bool bind(socket_t fd, const string &addr/*ip:port*/){
                address_info_t info;
                if (!parse_addr(addr, info))
                    return false;

                int opt = 1;
                setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
#if (defined SHINE_OS_WINDOWS)
#else
#if (defined SO_REUSEPORT)
                setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
#endif
#endif

                struct sockaddr_in address;
                address.sin_family = AF_INET;
                address.sin_port = htons(info.get_port());
                address.sin_addr.s_addr = inet_addr(info.get_ip().c_str());

                return ::bind(fd, (struct sockaddr *)&address, sizeof(address)) == 0;
            }

            /** 
             *@brief 侦听套接字
             *@param fd 套按字
             *@param backlog 队列大小
             *@return bool 
             *@warning 
             *@note 
            */
            static bool listen(socket_t fd, int32 backlog){
                return ::listen(fd, backlog) == 0;
            }

            /** 
             *@brief 接受一个新连接
             *@param fd 服务端套接字
             *@param new_fd 新连接套接字
             *@param info 新连接地址信息
             *@return bool 
             *@warning 
             *@note 
            */
            static bool accept(socket_t fd, socket_t &new_fd, address_info_t &info){

                struct sockaddr_in addr;
                socklen_t addr_len = sizeof(addr);
                auto tmp = ::accept(fd, (struct sockaddr*)&addr, &addr_len);
                if (tmp <= 0)
                    return false;

                new_fd = tmp;
                get_remote_addr(new_fd, info);

                return true;
            }

            /** 
             *@brief 套接字连接目标地址
             *@param fd 套接字
             *@param addr 目标地址
             *@param timeout 连接超时时间，单位毫秒
             *@return bool 
             *@warning 
             *@note 
            */
            static bool connect(socket_t fd, const string &addr, uint32 timeout)
            {
                address_info_t info;
                if (!parse_addr(addr, info))
                    return false;

                char ip[128];
                SHINE_SNPRINTF(ip, sizeof(ip)-1, "%s", info.get_ip().c_str());
                short port = info.get_port();

                void* svraddr = nullptr;
                int32 svraddr_len = 0;
                struct sockaddr_in svraddr_4;
                struct sockaddr_in6 svraddr_6;

                struct addrinfo *result;
                if (getaddrinfo(ip, NULL, NULL, &result) != 0)
                    return false;

                struct sockaddr *sa = result->ai_addr;
                socklen_t maxlen = sizeof(ip);

                if (sa->sa_family == AF_INET) {
                    char *tmp = (char *)ip;
                    if (inet_ntop(AF_INET, (void *)&(((struct sockaddr_in *) sa)->sin_addr), tmp, maxlen) == NULL)
                        return false;

                    svraddr_4.sin_family = AF_INET;
                    svraddr_4.sin_addr.s_addr = inet_addr(ip);
                    svraddr_4.sin_port = htons(port);
                    svraddr_len = sizeof(svraddr_4);
                    svraddr = &svraddr_4;
                }
                else if (sa->sa_family == AF_INET6) {
                    if (inet_ntop(AF_INET6, &(((struct sockaddr_in6 *) sa)->sin6_addr), ip, maxlen) != 0)
                        return false;

                    memset(&svraddr_6, 0, sizeof(svraddr_6));
                    svraddr_6.sin6_family = AF_INET6;
                    svraddr_6.sin6_port = htons(port);

                    if (inet_pton(AF_INET6, ip, &svraddr_6.sin6_addr) < 0)
                        return false;

                    svraddr_len = sizeof(svraddr_6);
                    svraddr = &svraddr_6;
                }

                freeaddrinfo(result);

                set_noblock(fd, true);

                bool ret = false;
                if (::connect(fd, (struct sockaddr*)svraddr, svraddr_len) != 0)
                {
                    int32 err = get_error();
#if (defined SHINE_OS_WINDOWS)
                    if (err != WSAEINPROGRESS && err != WSAEWOULDBLOCK) {
                        ret = false;
                    }
#else
                    if (err != EINPROGRESS && err != EWOULDBLOCK) {
                        ret = false;
                    }
#endif
                    else if (timeout > 0)
                    {
                        struct timeval tv;
                        tv.tv_sec = timeout / 1000;
                        tv.tv_usec = timeout % 1000;
                        fd_set wset, rset;
                        FD_ZERO(&wset);
                        FD_ZERO(&rset);
                        FD_SET(fd, &wset);
                        FD_SET(fd, &rset);

                        if (::select((int)fd + 1, &rset, &wset, NULL, &tv) == 1)
                            ret = FD_ISSET(fd, &wset) == 1 ? true : false;
                    }
                }
                else
                {
                    ret = true;
                }

                set_noblock(fd, false);
                return ret;
            }

            /** 
             *@brief 创建一对相互连接的套接字
             *@param pair 套接字对
             *@return bool 
             *@warning 
             *@note 
            */
            static bool create_socketpair(std::pair<socket_t, socket_t> &pair) {

#if (defined SHINE_OS_LINUX || defined SHINE_OS_ANDROID)
                int sockets[2];
                if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) < 0)
                    return false;

                pair.first = sockets[0];
                pair.second = sockets[1];
                return true;
#else


                struct addrinfo addr;
                struct addrinfo *paddr;
                memset(&addr, 0, sizeof(addr));

                addr.ai_family = AF_INET;
                addr.ai_socktype = SOCK_STREAM;
                addr.ai_protocol = 0;

                const int8* address = "127.0.0.1";

                if (getaddrinfo(address, "0", &addr, &paddr) != 0)
                    return false;

                socket_t l, s, c;
                int32 opt = 1;

                l = s = c = invalid_socket;
                l = create(addr.ai_family, addr.ai_socktype, addr.ai_protocol); 
                if (l == invalid_socket)
                    goto fail;

                ::setsockopt(l, SOL_SOCKET, SO_REUSEADDR, (const int8*)&opt, sizeof(opt));

                if (::bind(l, paddr->ai_addr, (int32)paddr->ai_addrlen) != 0
                    || ::getsockname(l, paddr->ai_addr, (int*)&paddr->ai_addrlen) != 0
                    || ::listen(l, 5) != 0)
                    goto fail;

                c = create(paddr->ai_family, paddr->ai_socktype, paddr->ai_protocol); 

                if (c == invalid_socket || ::connect(c, paddr->ai_addr, (int32)paddr->ai_addrlen) != 0)
                    goto fail;

                s = ::accept(l, 0, 0);

                if (s == invalid_socket)
                    goto fail;

                close(l);

                pair.first = c;
                pair.second = s;

                return true;

            fail:
                close(l);
                close(c);
                close(s);

                return false;
#endif
            }

            /** 
             *@brief 关闭套接字
             *@param fd 
             *@return void 
             *@warning 
             *@note 
            */
            static void close(socket_t fd)
            {
                if (fd == invalid_socket)
                    return;

#if (defined SHINE_OS_WINDOWS)
                ::closesocket(fd);
#else
                ::close(fd);
#endif
            }

            /** 
             *@brief 创建套接字
             *@param af 
             *@param type 
             *@param protocol 
             *@return socket_t 
             *@warning 
             *@note 当在windows下时创建支持iocp的套接字
            */
            static socket_t create(int32 af, int32 type, int32 protocol = 0)
            {
                init();
                socket_t s = invalid_socket;
#if (defined SHINE_OS_WINDOWS)
                af = PF_INET;
                s = WSASocket(af, type, protocol, 0, 0, WSA_FLAG_OVERLAPPED);
#else
                s = ::socket(af, type, protocol);
#endif
                return s;
            }

#if (defined SHINE_OS_WINDOWS)
            static bool load_wsa_func(GUID &guid, void *&pfun) {
                static once_flag t;
                static socket_t fd = invalid_socket;
                std::call_once(t, []{
                    fd = create(AF_INET, SOCK_STREAM, IPPROTO_TCP);
                });

                DWORD tmp = 0;
                return ::WSAIoctl(fd, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid
                    , sizeof(guid), &pfun, sizeof(pfun), &tmp, NULL, NULL) != SOCKET_ERROR;
            }
#endif

        private:
            static void init(){
#if (defined SHINE_OS_WINDOWS)
                static once_flag t;
                std::call_once(t, []{
                    WSADATA wsa_data;
                    WSAStartup(0x0201, &wsa_data);
                });
#endif
            }

        };

        class acceptor;
        class connection;
        class connector;

        struct task_data_block_t
        {
            size_t task_len = 0;
            const int8* data = nullptr;
            size_t data_len = 0;
            bool flush = false;
        } ;

        typedef std::function<void(connection *conn)> close_callback_t;
        typedef std::function<bool(connection *conn)> timeout_callback_t;
        typedef std::function<bool(const int8 *data, size_t len, connection *conn)> recv_callback_t;
        typedef std::function<bool(size_t len, connection *conn)> send_callback_t;

        struct context
#if (defined SHINE_OS_WINDOWS)
            : OVERLAPPED
#endif
        {
            context()
            {
#if (defined SHINE_OS_WINDOWS)
                memset(this, 0, sizeof(OVERLAPPED));
                set_status(context::e_idle);
#endif

            }

            void flush(size_t len) {
                _buf.erase(0, len);
            }

            enum status{
                e_idle = 1,
                e_accept = 2,
                e_connect = 4,
                e_recv = 8,
                e_send = 16,
                e_close = 32,
                e_exit = 64,
            };

            static const size_type _recv_some = 1024;
            const size_type get_recv_some() const{ return _recv_some; }

            SHINE_GEN_MEMBER_GETSET(shine::string, buf);

#if (defined SHINE_OS_WINDOWS)
            SHINE_GEN_MEMBER_GETSET(int32, status, = e_idle);
            SHINE_GEN_MEMBER_GETSET(WSABUF, WSABuf);
            
        public:
            typedef void* parent_t;
            SHINE_GEN_MEMBER_GETSET(parent_t, parent, = nullptr);
#endif

        };

        class proactor_engine;
        class peer {
        public:
            enum {
                e_acceptor = 0,
                e_connector = 1,
                e_connection = 2,
            };
        public:
            peer(){

            }
            virtual ~peer(){}


            SHINE_GEN_MEMBER_GETSET(string, name);
            SHINE_GEN_MEMBER_GETSET(socket_t, socket_fd, = invalid_socket);
            SHINE_GEN_MEMBER_GETSET(uint32, recv_timeout, = 0);
            SHINE_GEN_MEMBER_GETSET(int32, type);
#if (defined SHINE_OS_WINDOWS)
            SHINE_GEN_MEMBER_GETSET(HANDLE, kernel_fd, = nullptr);
#else
            SHINE_GEN_MEMBER_GETSET(socket_t, kernel_fd, = invalid_socket);
#endif
        public:
            typedef timer_manager* timer_manager_t;
            SHINE_GEN_MEMBER_GETSET(timer_manager_t, timer_manager);
        };

    }
}
