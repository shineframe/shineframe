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
#include "../common/define.hpp"
#include "../net/proactor_engine.hpp"
#include "../util/sha1.hpp"
#include "websocket_parser.hpp"

#if (defined SHINE_OS_WINDOWS)
#else
#endif

using namespace std;

namespace shine
{
    namespace websocket
    {
        typedef std::function<bool(frame_type type, const int8 *data, size_t len, net::connection *conn) > websocket_recv_callback_t;

        class server_peer{
        public:
            server_peer()
            {

            }
            ~server_peer(){

            }

            void run(net::connection *conn)
            {
                conn->set_recv_timeout(get_recv_timeout());

                conn->register_recv_callback(std::bind(&server_peer::base_recv_callback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
                conn->register_send_callback(std::bind(&server_peer::base_send_callback, this, std::placeholders::_1, std::placeholders::_2));
                conn->register_recv_timeout_callback(std::bind(&server_peer::base_recv_timeout_callback, this, std::placeholders::_1));
                conn->register_send_timeout_callback(std::bind(&server_peer::base_send_timeout_callback, this, std::placeholders::_1));
                conn->register_close_callback(std::bind(&server_peer::base_close_callback, this, std::placeholders::_1));

                conn->async_recv();
            }

        private:
            void send_websocket_response(net::connection *conn){
                    unsigned char digest[20]; // 160 bit sha1 digest

                    string answer;
                    answer += "HTTP/1.1 101 Switching Protocols\r\n";
                    answer += "Upgrade: WebSocket\r\n";
                    answer += "Connection: Upgrade\r\n";

                    if (get_key().length() > 0) {
                        string accept_key;
                        accept_key += get_key();
                        accept_key += "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"; //RFC6544_MAGIC_KEY

                        SHA1 sha;
                        sha.Input(accept_key.data(), (unsigned int)accept_key.size());
                        sha.Result((unsigned*)digest);

                        //little endian to big endian
                        for (int i = 0; i < 20; i += 4) {
                            unsigned char c;

                            c = digest[i];
                            digest[i] = digest[i + 3];
                            digest[i + 3] = c;

                            c = digest[i + 1];
                            digest[i + 1] = digest[i + 2];
                            digest[i + 2] = c;
                        }

                        string tmp;
                        tmp.assign((const char *)digest, 20);

                        answer += "Sec-WebSocket-Accept: " + string::encode_base64(tmp) + "\r\n";
                    }
                    if (get_protocol().length() > 0) {
                        answer += "Sec-WebSocket-Protocol: " + get_protocol() + "\r\n";
                    }
                    answer += "\r\n";

                    conn->async_send(answer.data(), answer.size());
            }
            bool assert_websocket_request(){
                if (get_http_request().get_method() != http::method::get)
                    return false;

                http::header::entrys_t &entrys = get_http_request().get_entrys();

                bool cond_1 = false;
                bool cond_2 = false;

                for (http::entry_t &entry : entrys)
                {
                    if (entry.get_key() == "Upgrade" && entry.get_value().to_lower() == "websocket")
                        cond_1 = true;

                    if (entry.get_key() == "Connection" && entry.get_value().to_lower() == "upgrade")
                        cond_2 = true;

                    if (entry.get_key() == "Origin")
                        set_origin(entry.get_value());

                    if (entry.get_key() == "Sec-WebSocket-Key")
                        set_key(entry.get_value());

                    if (entry.get_key() == "Sec-WebSocket-Protocol")
                        set_protocol(entry.get_value());
                }

                return cond_1 && cond_2;
            }

            bool base_recv_callback(const int8 *data, shine::size_t len, net::connection *conn)
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

                    if (get_is_first_package())
                    {
                        int8 *header_end = strstr((int8*)real_data, "\r\n\r\n");
                        if (header_end == NULL)
                        {
                            if (get_buf().size() - get_buf_pos() >= 1024)
                                return false;

                            return true;
                        }
                        else
                        {
                            get_is_first_package() = false;
                        }

                        shine::size_t cost_len = get_http_request().decode_header(real_data, header_end - real_data + 4);
                        if (cost_len < 0)
                            return false;

                        if (!assert_websocket_request())
                            return false;

                        send_websocket_response(conn);

                        get_buf_pos() += cost_len;
                        continue;
                    }

                    uint8 *out;
                    size_t out_len;
                    size_t cost_len;
                    auto ret = parser::decode((uint8*)get_buf().data() + get_buf_pos(), get_buf().size() - get_buf_pos(), out, out_len, cost_len);
                    if (ret == e_incomplete || ret == e_incomplete_text || ret == e_incomplete_binary)
                        return true;
                    else if (ret == e_error)
                        return false;
                    else
                    {
                        get_buf_pos() += cost_len;                        
                        
                        if (ret == e_ping)
                        {
                            shine::string pong = parser::encode(e_pong, 0, 0);
                            conn->async_send(pong.data(), pong.size());
                        }
                        else
                        {
                            if (get_recv_callback() != nullptr)
                            {
                                if (!get_recv_callback()(ret, (const int8*)out, out_len, conn))
                                    return false;
                            }
                        }
                    }
                }

                return true;
            }

            bool base_send_callback(shine::size_t len, net::connection *conn){
                if (get_send_callback() != nullptr)
                    return get_send_callback()(len, conn);
                return true;
            }

            bool base_recv_timeout_callback(net::connection *conn){
                if (get_recv_timeout_callback() != nullptr)
                    return get_recv_timeout_callback()(conn);
                return false;
            }

            bool base_send_timeout_callback(net::connection *conn){
                if (get_send_timeout_callback() != nullptr)
                    return get_send_timeout_callback()(conn);
                return true;
            }

            void base_close_callback(net::connection *conn)
            {
                if (get_close_callback() != nullptr)
                    return get_close_callback()(conn);
                delete this;
            }

        private:
            SHINE_GEN_MEMBER_GETSET(bool, is_first_package, = true);
            SHINE_GEN_MEMBER_GETSET(uint32, recv_timeout, = 0);
            SHINE_GEN_MEMBER_GETSET(shine::string, buf);
            SHINE_GEN_MEMBER_GETSET(shine::size_t, buf_pos, = 0);

            SHINE_GEN_MEMBER_GETSET(string, origin);
            SHINE_GEN_MEMBER_GETSET(string, protocol);
            SHINE_GEN_MEMBER_GETSET(string, key);

            SHINE_GEN_MEMBER_GETSET(http::request, http_request);
            SHINE_GEN_MEMBER_GETREG(websocket_recv_callback_t, recv_callback);
            SHINE_GEN_MEMBER_GETREG(net::close_callback_t, close_callback, = nullptr);
            SHINE_GEN_MEMBER_GETREG(net::timeout_callback_t, recv_timeout_callback, = nullptr);
            SHINE_GEN_MEMBER_GETREG(net::timeout_callback_t, send_timeout_callback, = nullptr);
            SHINE_GEN_MEMBER_GETREG(net::send_callback_t, send_callback, = nullptr);

        };

    }
}
