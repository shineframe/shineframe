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
#include "rpc_client.hpp"

#if (defined SHINE_OS_WINDOWS)
#else
#endif

using namespace std;


namespace shine
{
    namespace rpc
    {
        struct conn_session_t{
            uint8 decode_step = e_decode_header;
            package_t header;
            string buf;
            size_t buf_pos = 0;
            uint64 sequence = 0;
        };

        class server{
        public:
            typedef std::function <bool(const int8 *data, shine::size_t len, package_t &header, string &rsp_data)> rpc_handle_t;
        public:
            server(net::proactor_engine &engine, const string &name, const string &addr) : _engine(engine){
                _name = name;
                _addr = addr;
            }
            virtual ~server() {

            }

        public:
            void register_rpc_handle(uint64 type, rpc_handle_t func) {
                _handle_map.emplace(type, std::move(func));
            }

            void run(){
                if (!_run)
                {
                    init();
                    _run = true;
                }
            }
        private:
            void init(){
                _engine.add_acceptor(_name, _addr, [this](bool status, net::connection *conn)->bool{
                    if (status) {
                        conn_session_t *data = new conn_session_t;
                        conn->set_bind_data(data);
                        conn->set_recv_timeout(0);
                        conn->register_recv_callback(std::bind(&server::recv_callback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
                        conn->register_close_callback(std::bind(&server::close_callback, this, std::placeholders::_1));
                        conn->async_recv();
                    }

                    return true;
                });
            }

            bool recv_callback(const int8 *data, shine::size_t len, net::connection *conn)
            {
                conn_session_t *bind_data = (conn_session_t*)conn->get_bind_data();
                if (bind_data == nullptr)
                    return false;

                string &buf = bind_data->buf;
                size_t &buf_pos = bind_data->buf_pos;
                uint8 &decode_step = bind_data->decode_step;
                package_t &header = bind_data->header;

                buf.append(data, len);
                for (;;)
                {
                    if (buf.size() == buf_pos)
                    {
                        buf.clear();
                        buf_pos = 0;
                        return true;
                    }

                    if (decode_step == e_decode_header)
                    {
                        size_t cost_len = header.decode(buf.data() + buf_pos, buf.size() - buf_pos);

                        if (cost_len == 0)
                            return true;

                        buf_pos += cost_len;
                        decode_step = e_decode_body;
                    }

                    if (decode_step == e_decode_body)
                    {
                        if (buf.size() - buf_pos < header.length)
                            return true;

                        auto iter = _handle_map.find(header.identify);
                        if (iter != _handle_map.end())
                        {
                            package_t rsp_header;
                            string rsp_data;
                            bool rc = iter->second(buf.data() + buf_pos, header.length, rsp_header, rsp_data);

                            rsp_header.length = rsp_data.size();
                            string header_data = rsp_header.encode();

                            iovec_t iov[2];
                            iov[0].data = (int8 *)header_data.data();
                            iov[0].size = header_data.size();
                            iov[1].data = (int8 *)rsp_data.data();
                            iov[1].size = rsp_data.size();

                            conn->async_sendv(iov, 2);

                            if (!rc)
                                return false;
                        }
                        buf_pos += header.length;
                        decode_step = e_decode_header;
                        continue;
                    }
                }

                return true;
            }

            void close_callback(net::connection *conn){
                conn_session_t *bind_data = (conn_session_t*)conn->get_bind_data();
                if (bind_data != nullptr)
                    delete bind_data;
            }

        private:
            bool _run = false;
            string _name;
            string _addr;
            net::proactor_engine &_engine;
            std::unordered_map<uint64, rpc_handle_t> _handle_map;
        };

    }
}