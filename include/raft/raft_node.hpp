#pragma once

#include <iostream>
#include <thread>
#include <memory>
#include "common/macros.hpp"
#include "../filesystem/filesystem.hpp"
#include "../net/proactor_engine.hpp"
#include "raft_config.hpp"
#include "raft_protocol.hpp"
#include "../util/log.hpp"
#include "../rpc/rpc_server.hpp"

#if (defined SHINE_OS_WINDOWS)
#else
#endif

namespace shine
{
    namespace raft{
        using namespace net;

        enum {
            e_decode_header = 0,
            e_decode_body = 1,
            e_decode_done = 2,
        };

        enum {
            e_endpoint_server = 0,
            e_endpoint_client = 1,
        };

        enum {
            e_success = 0,
            e_failed = 1
        };

        struct request_session_t{
            uint64 identify;
            uint64 sequence;
            uint64 timer_id = invalid_timer_id;
            uint64 request_count;
            uint64 response_count = 0;
            uint64 pass_count = 0;
            uint64 unpass_count = 0;
        };

        struct conn_session_t{
            uint8 type;
            uint8 decode_step = e_decode_header;
            package_t header;
            string buf;
            size_t buf_pos = 0;
            uint64 sequence = 0;
        };


#define NODE_LOG(...) _log.write(_log_level, _log_output, __VA_ARGS__);
        class node{
        public:
            node() {
                _request_session.reserve(1000);
            }
            void run(const string &config_file_path){
                NODE_LOG("raft node running. config path : %s", config_file_path.c_str());
                _config_file_path = config_file_path;
                _thread = std::make_shared<std::thread>(std::bind(&node::worker_func, this));
            }

        private:
            bool recv_callback(const int8 *data, shine::size_t len, net::connection *conn)
            {
                conn_session_t *bind_data = (conn_session_t*)conn->get_bind_data();
                if (bind_data == nullptr)
                    return false;

                string &buf = bind_data->buf;
                size_t &buf_pos = bind_data->buf_pos;
                uint8 &decode_step = bind_data->decode_step;
                package_t &header = bind_data->header;
//                 uint8 &type = bind_data->type;

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

                        decode_step = e_decode_body;

                        buf_pos += cost_len;

                        if (buf.size() - buf_pos < header.length)
                            return true;
                    }

                    if (decode_step == e_decode_body)
                    {
                        if (!on_message(header.identify, buf.data() + buf_pos, header.length, conn))
                            return false;

                        buf_pos += header.length;
                        decode_step = e_decode_header;
                        continue;
                    }
                }

                return true;
            }

            bool send_callback(size_t len, connection *conn){
                return true;
            }

            bool recv_timeout_callback(connection *conn){
                //leader发生异常，启动投票定时器
                if (conn == _leader_peer)
                {
                    set_timer(_vote_timer, _config.vote_wait_base, std::bind(&node::handle_vote_timer, this));
                }
                return false;
            }

            bool send_timeout_callback(connection *conn){
                return true;
            }

            void close_callback(connection *conn){
                conn_session_t *bind_data = (conn_session_t*)conn->get_bind_data();
                if (bind_data != nullptr)
                {
                    if (bind_data->type == e_endpoint_server)
                    {
                        _server_remote_peers.erase(conn);
                    }
                    else
                    {
                        _client_peers.erase(conn->get_name());
                    }

                    delete bind_data;
                }
            }

            void register_callbacks(connection *conn){
                conn->register_recv_callback(std::bind(&node::recv_callback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
                conn->register_send_callback(std::bind(&node::send_callback, this, std::placeholders::_1, std::placeholders::_2));
                conn->register_recv_timeout_callback(std::bind(&node::recv_timeout_callback, this, std::placeholders::_1));
                conn->register_send_timeout_callback(std::bind(&node::send_timeout_callback, this, std::placeholders::_1));
                conn->register_close_callback(std::bind(&node::close_callback, this, std::placeholders::_1));
            }

            bool on_accept(bool status, connection *conn){
                if (status)
                {
                    conn->set_recv_timeout(_config.heartbeat * _config.heartbeat_timeout_count);
                    conn_session_t *data = new conn_session_t;
                    data->type = e_endpoint_server;
                    conn->set_bind_data(data);
                    register_callbacks(conn);
                    _server_remote_peers.insert(conn);
                    conn->async_recv();
                }

                return true;
            }
 
            void on_connect(bool status, connection *conn){
                if (status)
                {
                    conn->set_recv_timeout(_config.heartbeat * _config.heartbeat_timeout_count);
                    conn_session_t *data = new conn_session_t;
                    data->type = e_endpoint_client;
                    conn->set_bind_data(data);
                    _client_peers[conn->get_name()] = conn;
                    register_callbacks(conn);
                    conn->async_recv();
                }
                else
                {
//                     std::cout << conn->get_remote_addr().get_ip() << ":" << conn->get_remote_addr().get_port() << " connect failed!"<< endl;
                }
            }

            void worker_func(){
                file config;

                if (!config.open(_config_file_path))
                {
                    _log.init("raft_error", _log_level);
                    NODE_LOG("load config file failed. path: %s", _config_file_path.c_str());
                    return;
                }

                _log.init("raft_node_" + _config.id, _log_level);

                string data;
                config.readall(data);
                config.close();

                if (!_config.json_decode(data))
                {
                    NODE_LOG("parse config file failed. path: %s", _config_file_path.c_str());
                    return;
                }

                NODE_LOG("config: \n%s", data.c_str());

                if (!_status_file.open(_config.status_file))
                {
                    NODE_LOG("load status file failed. path: %s", _config.status_file.c_str());
                    return;
                }

                _status_file.readall(data);
                if (!_status.shine_serial_decode(data))
                {
                    NODE_LOG("parse status file failed. path: %s", _config.status_file.c_str());
                    return;
                }

                if (!_entry_file.open(_config.entry_file))
                {
                    NODE_LOG("load entry file failed. path: %s", _config.entry_file.c_str());
                    return;
                }

                _entry_file.readall(data);
                if (!_entry.shine_serial_decode(data))
                {
                    NODE_LOG("parse entry file failed. path: %s", _config.entry_file.c_str());
                    return;
                }

                _self_info.id = _config.id;

                for (auto iter : _config.nodes)
                {
                    string addr;

                    if (iter.first == _self_info.id)
                    {
                        _self_info = iter.second;
                        addr = "0.0.0.0:";
                        addr += iter.second.port;

                        _engine.add_acceptor("self:" + iter.second.id, addr, std::bind(&node::on_accept, this, std::placeholders::_1, std::placeholders::_2));
                    }

                    {
                        addr = iter.second.ip;
                        addr += ":";
                        addr += iter.second.port;

                        _engine.add_connector(iter.second.id, addr, std::bind(&node::on_connect, this, std::placeholders::_1, std::placeholders::_2));
                    }
                }

                set_timer(_heartbeat_timer, _config.heartbeat, std::bind(&node::handle_heartbeat_timer, this));
                set_timer(_vote_timer, _config.vote_wait_base, std::bind(&node::handle_vote_timer, this));
                _engine.run();
            }

        ///////////////////////////////////////////////////////////////////////////////////////
        private:
            bool on_message(size_t type, const int8 *data, size_t len, connection *conn){
                conn_session_t *sess = (conn_session_t*)conn->get_bind_data();
                if (sess == nullptr)
                    return false;

#define HANDLE_MESSAGE(MSG)  \
                if (type == MSG::identify)\
                {\
                MSG msg; \
                if (!msg.shine_serial_decode(data, len))\
                return false; \
                \
                if (msg.identify != raft_heartbeat::identify){\
                NODE_LOG("on_message:%s local:%s:%d, remote:%s:%d", typeid(MSG).name(), conn->get_local_addr().get_ip().c_str(), conn->get_local_addr().get_port()\
                    , conn->get_remote_addr().get_ip().c_str(), conn->get_remote_addr().get_port()); }\
                    return sess->type == e_endpoint_client ? handle_client_##MSG(msg, conn, sess) : handle_server_##MSG(msg, conn, sess); \
                }

                HANDLE_MESSAGE(raft_heartbeat);
                HANDLE_MESSAGE(raft_vote_request);
                HANDLE_MESSAGE(raft_vote_response);
                HANDLE_MESSAGE(raft_submit_request);
                HANDLE_MESSAGE(raft_submit_response);
                HANDLE_MESSAGE(raft_commit_request);
                HANDLE_MESSAGE(raft_commit_response);

                return true;
            }

            bool handle_heartbeat_timer(){
                raft_heartbeat msg;
                fill_header(msg.header);

                send(msg, &_server_remote_peers);
                return true;
            }

            bool handle_vote_timer(){
                if (_server_remote_peers.size() == 0)
                    return true;

                raft_vote_request msg;
                ++_status.term;
                fill_request_header(msg.header);

                request_session_t sess;
                sess.sequence = msg.header.sequence;
                sess.request_count = _server_remote_peers.size();
                sess.response_count = 0;

                send(msg, &_server_remote_peers);

                auto sequence = msg.header.sequence;
                set_timer(sess.timer_id, 3000, [this, sequence]()->bool{
                    handle_request_session_timeout(sequence);
                    return false;
                });

                _request_session.emplace(sequence, std::move(sess));
                return false;
            }

            bool handle_client_raft_heartbeat(raft_heartbeat &msg, connection *conn, conn_session_t *sess){
                //当leader发生变化时
                if (msg.header.leader && msg.header.term > _status.term && _leader_peer != conn)
                {
                    //此种情况一般发生在新节点刚启动时，或者旧leader通讯恢复时
                    cancel_timer(_vote_timer);
                    _leader_peer = conn;
                    _leader = false;

                    _status.term = msg.header.term;
                    save_status();
                }

                raft_heartbeat rsp;
                send(rsp, nullptr, conn);
                return true;
            }

            bool handle_server_raft_heartbeat(raft_heartbeat &msg, connection *conn, conn_session_t *sess){
                return true;
            }

            bool handle_client_raft_vote_request(raft_vote_request &msg, connection *conn, conn_session_t *sess){
                //处理投票请求
                raft_vote_response rsp;

                fill_response_header(msg.header, rsp.header);

                if (msg.header.id == _config.id || msg.header.uncommit > _status.uncommit){
                    rsp.result = e_success;
                }
                else {
                    rsp.result = e_failed;
                }

                send(rsp, nullptr, conn);
                return true;
            }

            bool handle_server_raft_vote_request(raft_vote_request &msg, connection *conn, conn_session_t *sess){
                //对端作为client无权发起此消息
                return false;
            }

            bool handle_client_raft_vote_response(raft_vote_response &msg, connection *conn, conn_session_t *sess){
                //对端作为server无权发起此消息
                return false;
            }

            bool handle_server_raft_vote_response(raft_vote_response &msg, connection *conn, conn_session_t *sess){
                auto iter = _request_session.find(msg.header.sequence);
                if (iter == _request_session.end())
                    return true;

                request_session_t &session = iter->second;
                msg.result == e_success ? ++session.pass_count : ++session.unpass_count;
                if (++session.response_count < session.request_count / 2)
                    return true;

                cancel_timer(session.timer_id);
                
                if (session.pass_count > session.unpass_count){
                    //成为leader
                    NODE_LOG("become leader, id : %s", _config.id.c_str());
                    _leader = true;
                    
                    save_status();

                    raft_heartbeat notify;
                    fill_request_header(notify.header);
                    send(notify, &_server_remote_peers);
                }
                else {
                    //没有成为leader
                }

                _request_session.erase(iter);
                return true;
            }

            bool handle_client_raft_submit_request(raft_submit_request &msg, connection *conn, conn_session_t *sess){
                return true;
            }

            bool handle_server_raft_submit_request(raft_submit_request &msg, connection *conn, conn_session_t *sess){
                //对端作为client无权发起此消息
                return true;
            }

            bool handle_client_raft_submit_response(raft_submit_response &msg, connection *conn, conn_session_t *sess){
                //对端作为server无权发起此消息
                return true;
            }

            bool handle_server_raft_submit_response(raft_submit_response &msg, connection *conn, conn_session_t *sess){
                return true;
            }

            bool handle_client_raft_commit_request(raft_commit_request &msg, connection *conn, conn_session_t *sess){
                return true;
            }

            bool handle_server_raft_commit_request(raft_commit_request &msg, connection *conn, conn_session_t *sess){
                //对端作为client无权发起此消息
                return true;
            }

            bool handle_client_raft_commit_response(raft_commit_response &msg, connection *conn, conn_session_t *sess){
                //对端作为server无权发起此消息
                return true;
            }

            bool handle_server_raft_commit_response(raft_commit_response &msg, connection *conn, conn_session_t *sess){
                return true;
            }

            void set_timer(uint64 &timer_id, uint64 delay, std::function<bool()> func){
                cancel_timer(timer_id);
                timer_id = _engine.get_timer().set_timer(delay, func);
            }

            void cancel_timer(uint64 &timer_id){
                _engine.get_timer().cancel_timer(timer_id);
                timer_id = invalid_timer_id;
            }

            void handle_request_session_timeout(uint64 sequence){
                std::unordered_map<uint64, request_session_t>::iterator iter = _request_session.find(sequence);
                if (iter == _request_session.end())
                    return;

                request_session_t &sess = iter->second;

                if (sess.identify == raft_vote_request::identify)
                {

                }
            }

            bool check_session_response(uint64 sequence, bool pass){

                std::unordered_map<uint64, request_session_t>::iterator iter = _request_session.find(sequence);
                if (iter == _request_session.end())
                    return true;

                request_session_t &sess = iter->second;
                pass ? ++sess.pass_count : ++sess.unpass_count;
                if (++sess.response_count >= sess.request_count / 2)
                {
                    cancel_timer(sess.timer_id);
                    _request_session.erase(iter);
                    return sess.pass_count > sess.unpass_count;
                }

                return false;
            }

            template<typename T>
            void send(const T &msg, std::set<connection*> *peers = nullptr, connection *peer = nullptr){
                if (peers == nullptr && peer == nullptr)
                    return;                    

                string msg_data = msg.shine_serial_encode();

                package_t header;
                header.identify = msg.identify;
                header.length = msg_data.size();
                string header_data = header.encode();

                iovec_t iov[2];
                iov[0].data = (int8 *)header_data.data();
                iov[0].size = header_data.size();
                iov[1].data = (int8 *)msg_data.data();
                iov[1].size = msg_data.size();

                if (peers)
                {
                    for (auto iter : *peers)
                    {
                        iter->async_sendv(iov, 2);
                        if (msg.identify != raft_heartbeat::identify)
                        {
                            NODE_LOG("send_message:%s local:%s:%d, remote:%s:%d"
                                , typeid(T).name()
                                , iter->get_local_addr().get_ip().c_str()
                                , iter->get_local_addr().get_port()
                                , iter->get_remote_addr().get_ip().c_str()
                                , iter->get_remote_addr().get_port());
                        }
                    }
                }

                if (peer)
                {
                    peer->async_sendv(iov, 2);
                    if (msg.identify != raft_heartbeat::identify)
                    {
                        NODE_LOG("send_message:%s local:%s:%d, remote:%s:%d"
                            , typeid(T).name()
                            , peer->get_local_addr().get_ip().c_str()
                            , peer->get_local_addr().get_port()
                            , peer->get_remote_addr().get_ip().c_str()
                            , peer->get_remote_addr().get_port());
                    }

                }
            }

            void fill_header(raft_header &header){
                header.id = _config.id;
                header.leader = _leader;
                header.term = _status.term;
                header.commit = _status.commit;
                header.uncommit = _status.uncommit;
            }

            void fill_request_header(raft_header &header){
                fill_header(header);
                header.sequence = ++_sequence;
            }

            void fill_response_header(const raft_header &req, raft_header &rsp){
                fill_header(rsp);
                rsp.sequence = req.sequence;
            }

            void save_status(){
                _status_file.save(_status.shine_serial_encode());
            }
        public:
            uint64 _sequence = 0;
            raft_config _config;
            string _config_file_path;
            raft_node_info _self_info;
            raft_node_status _status;
            file _status_file;

            raft_node_entry _entry;
            file _entry_file;

            net::proactor_engine _engine;
            std::shared_ptr<std::thread> _thread;
            std::set<connection*> _server_remote_peers;
            std::map<string, connection*> _client_peers;
            connection * _leader_peer = nullptr;
            uint64 _heartbeat_timer = invalid_timer_id;
            uint64 _vote_timer = invalid_timer_id;
            bool _leader = false;

            std::unordered_map<uint64, request_session_t> _request_session;

            log _log;
            int32 _log_level = log::e_debug;
            int32 _log_output = log::e_console | log::e_file;

        };
    }
}
