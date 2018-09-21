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
#include <condition_variable>
#include <atomic>
#include "../common/define.hpp"
#include "../net/socket.hpp"
#include "../net/proactor_engine.hpp"
#include "../util/tool.hpp"

#if (defined SHINE_OS_WINDOWS)
#else
#endif

using namespace std;


namespace shine
{
    namespace rpc
    {
        enum {
            e_decode_header = 0,
            e_decode_body = 1
        };

        class client{
        public:
            client(const string &addr) : _addr(addr){
            }
            virtual ~client() {
                close_connection();
            }

            template<typename REQ, typename RSP>
            void call(uint64 identify, const REQ &req, RSP &rsp, bool &result, uint32 timeout = 3000) {
                if (!reset_socket(timeout)) {
                    result = false;
                    return;
                }

                std::string body_data = req.shine_serial_encode();
                package_t package;
                package.identify = identify;
                package.length = body_data.size();
                std::string package_data = package.encode();

                package_data.append(std::move(body_data));

                size_t len = package_data.size();
                int rc = 0;

                while (len > 0)
                {
                    rc = ::send(_socket_fd, package_data.data(), (int)package_data.size(), 0);

                    if (rc <= 0) {
                        close_connection();
                        result = false;
                        return;
                    }
                    len -= rc;
                }

                std::string recv_buf;
                size_t recv_pos = 0;
                recv_buf.resize(32);
                int32 decode_step = e_decode_header;

                for (;;)
                {
                    rc = ::recv(_socket_fd, (int8*)recv_buf.data() + recv_pos, (int)(recv_buf.size() - recv_pos), 0);
                    if (rc <= 0)
                    {
                        close_connection();
                        result = false;
                        return;
                    }

                    if (decode_step == e_decode_header)
                    {
                        size_t header_len = package.decode(recv_buf.data() + recv_pos, recv_buf.size() - recv_pos);
                        if (header_len == 0) {
                            result = false;
                            return;
                        }
                        
                        recv_buf.erase(0, header_len);
                        recv_buf.resize(package.length);
                        recv_pos = 0;
                        rc -= (int)header_len;
                        decode_step = e_decode_body;
                    }

                    if (decode_step == e_decode_body)
                    {
                        recv_pos += rc;
                        if (recv_pos < package.length)
                            continue;

                        result = rsp.shine_serial_decode(recv_buf.data(), recv_buf.size());
                        return;
                    }
                }
            }

        private:
            bool reset_socket(uint32 timeout){
                if (_socket_fd == invalid_socket)
                {
                    _socket_fd = net::socket::create(AF_INET, SOCK_STREAM, 0);

                    if (_socket_fd == invalid_socket)
                        return false;

                    if (_addr.find(":") == string::npos)
                        return false;

                    if (!net::socket::connect(_socket_fd, _addr, timeout))
                    {
                        close_connection();
                        return false;
                    }

                    net::socket::set_nodelay(_socket_fd);
                }

                if (timeout > 0)
                {
#if (defined SHINE_OS_WINDOWS)
                    setsockopt(_socket_fd, SOL_SOCKET, SO_SNDTIMEO, (const char *)&timeout, sizeof(timeout));
                    setsockopt(_socket_fd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout));
#else
                    timeval tv;
                    tv.tv_sec = timeout / 1000;
                    tv.tv_usec = timeout % 1000;

                    setsockopt(_socket_fd, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv, sizeof(struct timeval));
                    setsockopt(_socket_fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(struct timeval));
#endif
                }

                return true;
            }
            void close_connection()
            {
                if (_socket_fd != invalid_socket)
                {
                    net::socket::close(_socket_fd);
                    _socket_fd = invalid_socket;
                }
            }

        protected:
            socket_t _socket_fd = invalid_socket;
            string _addr;
        };

        class pipe_client : public client{
        public:
            pipe_client(socket_t socket) : client("") {
                _socket_fd = socket;
            }
        };
    }
}