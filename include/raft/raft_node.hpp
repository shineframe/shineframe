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
#include "../concurrent_queue/concurrent_queue.hpp"

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
            e_endpoint_pipe_server = 2,
            e_endpoint_pipe_client = 3,
        };

        enum {
            e_success = 0,
            e_failed = 1
        };

        enum {
            e_status_boot = 0,
            e_status_vote = 1,
            e_status_get_entry = 2,
            e_status_leader = 3,
            e_status_follow = 4,
        };

        struct rpc_channel_t {
            net::connection *server;
            std::shared_ptr<rpc::pipe_client> client;
        };

        struct request_session_t{
            uint64 identify;
            uint64 sequence;
            uint64 timer_id = invalid_timer_id;
            uint64 request_count;
            uint64 response_count = 0;
            uint64 pass_count = 0;
            uint64 unpass_count = 0;
            raft_entry entry;
            net::connection *conn = nullptr;
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
        typedef std::function<void(std::shared_ptr<request_session_t> session)> session_func_t;
        typedef std::function<void(std::string &req, std::string *rsp)> raft_execute_func_t;
//         typedef std::function<void(raft_execute_request &req, raft_execute_response &rsp)> raft_execute_func_t;

        class node{
        public:
            node(const char *config_file_path, raft_execute_func_t execute_func){
                _config_file_path = config_file_path;
                _execute_func = std::move(execute_func);
                _thread = std::make_shared<std::thread>(std::bind(&node::worker_func, this));
            }

            bool execute(const char *data, std::size_t len, std::string &output, bool persistent = true){
                raft_execute_request req;
                raft_execute_response rsp;
                req.persistent = persistent;
                req.data.assign(data, len);

                execute(req, rsp);
                output = std::move(rsp.data);
                return rsp.result == raft_error::success;
            }

            bool execute(const std::string &input, std::string &output, bool persistent = true){
                raft_execute_request req;
                raft_execute_response rsp;
                req.persistent = persistent;
                req.data = input;

                execute(req, rsp);
                output = std::move(rsp.data);
                return rsp.result == raft_error::success;
            }

        private:
            uint64 gen_sequence(){
                uint64 ret = ((uint64)_config.type << 48 ) | ((uint64)_config.id << 32) | (++_sequence) ;
                return ret;
            }

            void execute(raft_execute_request &req, raft_execute_response &rsp) {

                int32 thread_id = (int32)GETTID();
                if (thread_id == _thread_id)
                {
                    rsp.result = raft_error::unable_execute;
                    return;
                }

                std::unordered_map<int32, std::shared_ptr<rpc_channel_t> >::iterator iter;
                {
                    std::unique_lock<std::recursive_mutex> lock(_execute_mutex);
                    iter = _execute_cannels.find(thread_id);
                    if (iter == _execute_cannels.end())
                    {
                        std::pair<socket_t, socket_t> pair;
                        net::socket::create_socketpair(pair);
                        std::shared_ptr<rpc_channel_t> rpc_cannel = std::make_shared<rpc_channel_t>();
                        _engine.add_connection("", pair.first, [&rpc_cannel, this](bool status, net::connection *conn)->bool{
                            if (status) {
                                rpc_cannel->server = conn;

                                conn_session_t *data = new conn_session_t;
                                data->type = e_endpoint_pipe_server;
                                conn->set_bind_data(data);
                                conn->set_recv_timeout(0);
                                conn->register_recv_callback(std::bind(&node::recv_callback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
                                conn->register_close_callback(std::bind(&node::close_callback, this, std::placeholders::_1));
                                conn->async_recv();
                            }
                            return true;
                        });

                        rpc_cannel->client = std::make_shared<rpc::pipe_client>(pair.second);

                        iter = _execute_cannels.emplace(std::move(thread_id), std::move(rpc_cannel)).first;
                    }

                }

                std::shared_ptr<rpc_channel_t> channel = iter->second;
                bool result = false;
                fill_request_header(req.header);
                channel->client->call(req.identify, req, rsp, result, 0);
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
                        if (conn == _leader_peer)
                        {
                            NODE_LOG("on leader disconnect.");

                            _leader_peer = nullptr;
                            if (_leader)
                                _leader = false;
                            set_timer(_vote_timer, _config.vote_wait_base, std::bind(&node::handle_vote_timer, this));
                        }
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

                    raft_heartbeat req;
                    fill_request_header(req.header);
                    send(req, nullptr, conn);
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
                _thread_id = (int32)GETTID();
                
                _request_session.reserve(1000);


                file config;

                if (!config.open(_config_file_path))
                {
                    _log.init("raft_error", _log_level);
                    NODE_LOG("load config file failed. path: %s", _config_file_path.c_str());
                    return;
                }

                string data;
                config.readall(data);
                config.close();

                if (!_config.json_decode(data))
                {
                    _log.init("raft_error", _log_level);
                    NODE_LOG("parse config file failed. path: %s", _config_file_path.c_str());
                    return;
                }

                _log.init("raft_node_" + std::to_string(_config.id), _log_level);
                NODE_LOG("raft node running. config path : %s", _config_file_path.c_str());

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
                if (!parse_entry(data))
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

                    if (iter.first != _self_info.id)
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
            bool parse_entry(string &data){
                size_t cost_len = 0;
                for (; cost_len < data.size(); )
                {
                    raft_entry entry;
                    if (entry.shine_serial_decode(data.data(), data.size(), cost_len)){
                        _commit_entry.entrys.emplace_back(std::move(entry));
                        _execute_func(entry.data, nullptr);
                    }
                    else{
                        return false;
                    }
                }
                
                return true;
            }

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
                /*NODE_LOG("recv from:%d type:%s sequence:%lld term:%llu commit:%llu min_uncommit:%llu max_uncommit:%llu", msg.header.id, std::string(typeid(msg).name()).substr(7).c_str(), msg.header.sequence, msg.header.term, msg.header.commit, msg.header.min_uncommit, msg.header.max_uncommit);*/}\
                return sess->type == e_endpoint_client ? handle_client_##MSG(msg, conn, sess) : handle_server_##MSG(msg, conn, sess); \
                }

                if (type == raft_execute_request::identify)
                {
                    return handle_execute_request(data, len, conn, sess);
                }
                else if (type == raft_execute_response::identify)
                {
                    raft_execute_response rsp;
                    if (!rsp.shine_serial_decode(data, len))
                        return false;

                    return handle_execute_response(rsp, conn, sess);
                }

                HANDLE_MESSAGE(raft_heartbeat);
                HANDLE_MESSAGE(raft_vote_request);
                HANDLE_MESSAGE(raft_vote_response);
                HANDLE_MESSAGE(raft_submit_request);
                HANDLE_MESSAGE(raft_submit_response);
                HANDLE_MESSAGE(raft_commit_request);
                HANDLE_MESSAGE(raft_commit_response);
                HANDLE_MESSAGE(raft_get_entry_request);
                HANDLE_MESSAGE(raft_get_entry_response);

                return true;
            }

            bool handle_heartbeat_timer(){
                raft_heartbeat msg;
                fill_header(msg.header);

                send(msg, &_server_remote_peers);
                return true;
            }

            void on_become_leader(){
                //成为leader
                NODE_LOG("become leader, type:%d, id:%d", _config.type, _config.id);

                _leader = true;
                _leader_peer = nullptr;
                bool save = false;
                for (size_t i = 0; i < _uncommit_entry.entrys.size(); i++)
                {
                    if (_uncommit_entry.entrys[i].no == get_commit_id() + 1)
                    {
                        save = true;
                        _execute_func(_uncommit_entry.entrys[i].data, nullptr);
                        _commit_entry.entrys.push_back(_uncommit_entry.entrys[i]);
                        _entry_file.append(_uncommit_entry.entrys[i].shine_serial_encode());
                    }
                }

                if (save)
                    _entry_file.save();

                _uncommit_entry.entrys.clear();
                handle_heartbeat_timer();
            }

            bool handle_vote_timer(){
                if (_server_remote_peers.size() == 0){
                    on_become_leader();
                    return false;
                }

                raft_vote_request msg;
                ++_status.term;
                save_status();
                fill_request_header(msg.header);

                std::shared_ptr<request_session_t> sess = std::make_shared<request_session_t>();
                sess->identify = msg.identify;
                sess->sequence = msg.header.sequence;
                sess->request_count = _server_remote_peers.size();
                sess->response_count = 0;

                send(msg, &_server_remote_peers);

                auto sequence = sess->sequence;
                set_timer(_vote_timer, _config.vote_wait_base, [this, sequence]()->bool{
                    handle_request_session_timeout(sequence);
                    return false;
                });

                sess->timer_id = _vote_timer;

                _request_session.emplace(sequence, std::move(sess));
                return false;
            }

            bool handle_execute_request(const int8 *data, size_t len, connection *conn, conn_session_t *sess){
                raft_execute_request req;
                if (!req.shine_serial_decode(data, len))
                    return false;

                //raft_execute_request &msg = req;
                //NODE_LOG("recv from:%d type:%s sequence:%llu term:%llu commit:%llu min_uncommit:%llu max_uncommit:%llu", msg.header.id, std::string(typeid(msg).name()).substr(7).c_str(), msg.header.sequence, msg.header.term, msg.header.commit, msg.header.min_uncommit, msg.header.max_uncommit);

                //leader节点
                if (_leader)
                {
                    if (req.persistent)
                    {
                        raft_submit_request submit_req;
                        submit_req.entry.no = get_next_uncommit_id();
                        submit_req.entry.data = req.data;

                        if (_server_remote_peers.size() > 0)
                        {
                            fill_request_header(submit_req.header);
                            send(submit_req, &_server_remote_peers);

                            std::shared_ptr<request_session_t> req_sess = std::make_shared<request_session_t>();
                            req_sess->conn = conn;
                            req_sess->identify = raft_submit_request::identify;
                            req_sess->request_count = _server_remote_peers.size();
                            req_sess->sequence = req.header.sequence;
                            req_sess->entry = submit_req.entry;

                            uint64 sequence = submit_req.header.sequence;
                            set_timer(req_sess->timer_id, _config.submit_wait_base, [this, sequence]()->bool{
                                handle_request_session_timeout(sequence);
                                return false;
                            });

                            _request_session.emplace(sequence, std::move(req_sess));
                        }
                        else{
                            raft_execute_response rsp;
                            fill_response_header(req.header, rsp.header);
                            _execute_func(req.data, &rsp.data);

                            std::string data = submit_req.entry.shine_serial_encode();
                            _entry_file.append(std::move(data));
                            _entry_file.save();
                            _commit_entry.entrys.push_back(submit_req.entry);

                            send(rsp, nullptr, conn);
                        }
                    }
                    else
                    {
                        raft_execute_response rsp;
                        fill_response_header(req.header, rsp.header);
                        _execute_func(req.data, &rsp.data);
                        send(rsp, nullptr, conn);
                    }                    
                }
                //不是leader则将请求转发至leader处理
                else
                {
                    if (_leader_peer != nullptr)
                    {
                        std::shared_ptr<request_session_t> req_sess = std::make_shared<request_session_t>();
                        req_sess->conn = conn;
                        req_sess->identify = raft_execute_request::identify;
                        req_sess->request_count = 1;
                        req_sess->sequence = req.header.sequence;

                        uint64 sequence = req_sess->sequence;
                        set_timer(req_sess->timer_id, _config.submit_wait_base, [this, sequence]()->bool{
                            handle_request_session_timeout(sequence);
                            return false;
                        });

                        _request_session.emplace(sequence, std::move(req_sess));
                        send(req, nullptr, _leader_peer);
                    }
                    else {
                        raft_execute_response rsp;
                        fill_response_header(req.header, rsp.header);
                        rsp.result = raft_error::submit_failed;
                        send(rsp, nullptr, conn);
                    }
                }

                return true;
            }

            bool handle_execute_response(raft_execute_response &msg, connection *conn, conn_session_t *sess){
                //NODE_LOG("recv from:%d type:%s sequence:%llu term:%llu commit:%llu min_uncommit:%llu max_uncommit:%llu", msg.header.id, std::string(typeid(msg).name()).substr(7).c_str(), msg.header.sequence, msg.header.term, msg.header.commit, msg.header.min_uncommit, msg.header.max_uncommit);

                auto pass_func = [&msg, this](std::shared_ptr<request_session_t> session){
                    send(msg, nullptr, session->conn);
                };

                auto unpass_func = pass_func;


                check_request_session(msg, pass_func, unpass_func);

                return true;
            }

            bool handle_client_raft_heartbeat(raft_heartbeat &msg, connection *conn, conn_session_t *sess){
                //当leader发生变化时
                if (msg.header.leader && _leader_peer != conn &&  msg.header.term >= _status.term)
                {
                    //成为follow
                    NODE_LOG("become follow, type:%d, id:%d", _config.type, _config.id);
                    //此种情况一般发生在新节点刚启动时，或者旧leader通讯恢复时
                    cancel_timer(_vote_timer);
                    _leader_peer = conn;
                    _leader = msg.header.id == _self_info.id;

                    _status.term = msg.header.term;
                    save_status();
                    send_raft_get_entry_request(conn);
                }

                raft_heartbeat rsp;
                send(rsp, nullptr, conn);
                return true;
            }

            bool handle_server_raft_heartbeat(raft_heartbeat &msg, connection *conn, conn_session_t *sess){
                return true;
            }

            bool is_serial(uint64 commit, uint64 min_commit){
                if (commit == 0)
                    return min_commit <= 1;

                return min_commit == 0 || min_commit == commit + 1;
            }

            bool handle_client_raft_vote_request(raft_vote_request &msg, connection *conn, conn_session_t *sess){
                //处理投票请求
                raft_vote_response rsp;

                fill_response_header(msg.header, rsp.header);

                uint64 commit = get_commit_id();
                uint64 min_uncommit = get_min_uncommit_id();
                uint64 max_uncommit = get_max_uncommit_id();

                bool self_serial = is_serial(commit, min_uncommit);
                bool other_serial = is_serial(msg.header.commit, msg.header.min_uncommit);

                if (msg.header.id == _config.id){
                    rsp.result = raft_error::success;
                }
                else if (self_serial) {
                    if (other_serial) {
                        if (msg.header.max_uncommit > max_uncommit)
                            rsp.result = raft_error::success;
                        else if (msg.header.max_uncommit == max_uncommit)
                            rsp.result = msg.header.id < _self_info.id ? raft_error::success : raft_error::disagree;
                        else
                            rsp.result = raft_error::disagree;
                    }
                    else{
                        rsp.result = msg.header.commit >= max_uncommit ? raft_error::success : raft_error::disagree;
                    }
                }
                else if (!self_serial) {
                    if (other_serial)
                        rsp.result = msg.header.max_uncommit >= commit ? raft_error::success : raft_error::disagree;
                    else if (msg.header.commit == commit)
                        rsp.result = msg.header.id < _self_info.id ? raft_error::success : raft_error::disagree;
                    else 
                        rsp.result = msg.header.commit > commit ? raft_error::success : raft_error::disagree;
                }

                if (rsp.result == raft_error::success)
                    cancel_timer(_vote_timer);

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

                session_func_t pass_func = [this](std::shared_ptr<request_session_t> session){
                    on_become_leader();
                };

                session_func_t unpass_func = [this](std::shared_ptr<request_session_t> session){
                    //未成为leader
                    _leader = false;
                };
                check_request_session(msg, pass_func, unpass_func);

                return true;
            }

            bool handle_client_raft_submit_request(raft_submit_request &msg, connection *conn, conn_session_t *sess){
//                 NODE_LOG("submit no:%llu", msg.entry.no);

                raft_submit_response rsp;
                rsp.no = msg.entry.no;
                rsp.result = raft_error::success;
                if (!_leader) {
                    if (conn == _leader_peer)
                    {
                        uint64 commit_id = get_commit_id();
                        uint64 min_uncommit_id = get_min_uncommit_id();
                        uint64 max_uncommit_id = get_max_uncommit_id();

                        if (min_uncommit_id == 0 && msg.entry.no > commit_id)
                        {
                            _uncommit_entry.entrys.push_back(msg.entry);
                        }
                        else if (min_uncommit_id > 0)
                        {
                            if (msg.entry.no == max_uncommit_id + 1)
                                _uncommit_entry.entrys.push_back(msg.entry);
                            else if (msg.entry.no > max_uncommit_id + 1)
                            {
                                _uncommit_entry.entrys.clear();
                                _uncommit_entry.entrys.push_back(msg.entry);
                            }
                        }

                        if (get_min_uncommit_id() > commit_id + 1 && !_get_entry_process)
                            send_raft_get_entry_request(_leader_peer);
                    }
                    else
                        rsp.result = raft_error::unable_execute;
                }

                fill_response_header(msg.header, rsp.header);
                send(rsp, nullptr, conn);
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
                auto pass_func = [&msg, this](std::shared_ptr<request_session_t> session){
                    raft_commit_request commit_req;
                    fill_request_header(commit_req.header);
                    commit_req.no = msg.no;

                    session->identify = raft_commit_request::identify;
                    session->request_count = _server_remote_peers.size();

                    send(commit_req, &_server_remote_peers);
                    _request_session.emplace(commit_req.header.sequence, std::move(session));
                };

                auto unpass_func = [&msg, this](std::shared_ptr<request_session_t> session){
                    raft_execute_response rsp;
                    fill_header(rsp.header);
                    rsp.header.sequence = session->sequence;
                    rsp.result = raft_error::submit_failed;
                    send(rsp, nullptr, session->conn);
                };


                check_request_session(msg, pass_func, unpass_func);
                return true;
            }

            bool handle_client_raft_commit_request(raft_commit_request &msg, connection *conn, conn_session_t *sess){
//                 NODE_LOG("commit no:%llu", msg.no);
                raft_commit_response rsp;
                rsp.no = msg.no;
                rsp.result = raft_error::success;

                if (!_leader) {
                    if (conn == _leader_peer)
                    {
                        bool save = false;
                        for (; _uncommit_entry.entrys.size() > 0 && get_commit_id() + 1 == get_min_uncommit_id() && get_min_uncommit_id() <= msg.no; )
                        {
                            save = true;
                            std::string &execute_req = _uncommit_entry.entrys[0].data;
                            std::string execute_rsp;
                            _execute_func(execute_req, &execute_rsp);

                            _commit_entry.entrys.push_back(_uncommit_entry.entrys[0]);
                            _entry_file.append(_uncommit_entry.entrys[0].shine_serial_encode());
                            _uncommit_entry.entrys.pop_front();
                        }
                        
                        if (save)
                            _entry_file.save();

                        if (get_commit_id() < msg.no)
                            send_raft_get_entry_request(conn);
                    }
                    else
                        rsp.result = raft_error::unable_execute;
                }

                fill_response_header(msg.header, rsp.header);
                send(rsp, nullptr, conn);
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
                auto pass_func = [&msg, this](std::shared_ptr<request_session_t> session){

                    std::string data = session->entry.shine_serial_encode();
//                     auto hex_data = string::print_hex_string(data);
//                     NODE_LOG("entry file old size:%lld, data size:%lld", _entry_file.size(), data.size());
//                     NODE_LOG("hex data:%s", hex_data.c_str());
                    _entry_file.append(std::move(data));
                    _entry_file.save();
//                     NODE_LOG("entry file new size:%lld", _entry_file.size());

                    _commit_entry.entrys.push_back(session->entry);

                    raft_execute_response rsp;
                    fill_header(rsp.header);
                    rsp.header.sequence = session->sequence;

                    _execute_func(session->entry.data, &rsp.data);

                    send(rsp, nullptr, session->conn);
                };

                auto unpass_func = [&msg, this](std::shared_ptr<request_session_t> session){
                    raft_execute_response rsp;
                    fill_header(rsp.header);
                    rsp.header.sequence = session->sequence;
                    rsp.result = raft_error::commit_failed;
                    send(rsp, nullptr, session->conn);
                };

                check_request_session(msg, pass_func, unpass_func);
                return true;
            }

            bool handle_client_raft_get_entry_request(raft_get_entry_request &msg, connection *conn, conn_session_t *sess){
                return true;
            }

            bool handle_server_raft_get_entry_request(raft_get_entry_request &msg, connection *conn, conn_session_t *sess){
                raft_get_entry_response rsp;
                if (_leader)
                {
                    rsp.result = raft_error::success;
                    for (size_t i = msg.header.commit + 1; i <= _commit_entry.entrys.size(); i++)
                    {
                        rsp.entrys.push_back(_commit_entry.entrys[i - 1]);
                        if (rsp.entrys.size() == 100)
                            break;
                    }
                }
                else
                {
                    rsp.result = raft_error::unable_execute;
                }

                send(rsp, nullptr, conn);
                return true;
            }

            bool handle_client_raft_get_entry_response(raft_get_entry_response &msg, connection *conn, conn_session_t *sess){
                auto pass_func = [&msg, this](std::shared_ptr<request_session_t> session){
                    cancel_timer(_get_entry_timer);
                    _get_entry_process = false;
                };

                auto unpass_func = pass_func;

                check_request_session(msg, pass_func, unpass_func);

                bool save = false;
                for (size_t i = 0; i < msg.entrys.size(); i++)
                {
                    if (msg.entrys[i].no == get_commit_id() + 1)
                    {
                        save = true;
                        std::string &a = msg.entrys[i].data;
                        _execute_func(a, nullptr);
                        _commit_entry.entrys.push_back(msg.entrys[i]);
                        _entry_file.append(msg.entrys[i].shine_serial_encode());
                    }
                }

                if (save){
                    save_status();
                    _entry_file.save();
                }

                while (get_min_uncommit_id() > 0 && get_min_uncommit_id() <= get_commit_id())
                    _uncommit_entry.entrys.pop_front();

                if (msg.header.commit > get_commit_id())
                    send_raft_get_entry_request(conn);
                return true;

            }

            bool handle_server_raft_get_entry_response(raft_get_entry_response &msg, connection *conn, conn_session_t *sess){
                return true;
            }

            uint64 get_commit_id() const{
                if (_commit_entry.entrys.size() == 0)
                    return 0;

                return _commit_entry.entrys[_commit_entry.entrys.size() - 1].no;
            }

            uint64 get_min_uncommit_id() const{
                if (_uncommit_entry.entrys.size() == 0)
                    return 0;

                return _uncommit_entry.entrys[0].no;
            }

            uint64 get_max_uncommit_id() const{
                if (_uncommit_entry.entrys.size() == 0)
                    return 0;

                return _uncommit_entry.entrys[_uncommit_entry.entrys.size() - 1].no;
            }

            uint64 get_next_uncommit_id() const{
                uint64 id = get_max_uncommit_id();
                if (id == 0)
                    return get_commit_id() + 1;

                return id + 1;
            }

            void send_raft_get_entry_request(net::connection *conn){
                raft_get_entry_request req;
                fill_request_header(req.header);

                std::shared_ptr<request_session_t> sess = std::make_shared<request_session_t>();
                sess->identify = req.identify;
                sess->sequence = req.header.sequence;
                sess->request_count = 1;
                sess->response_count = 0;

                auto sequence = sess->sequence;
                set_timer(_get_entry_timer, _config.get_entry_wait_base, [this, sequence]()->bool{
                    _get_entry_process = false;
                    return false;
                });

                sess->timer_id = _get_entry_timer;

                _request_session.emplace(sequence, std::move(sess));

                send(req, nullptr, conn);
                _get_entry_process = true;
            }

            void set_timer(uint64 &timer_id, uint64 delay, std::function<bool()> func){
                cancel_timer(timer_id);
                timer_id = _engine.get_timer().set_timer(delay, func);
            }

            void cancel_timer(uint64 &timer_id){
                _engine.get_timer().cancel_timer(timer_id);
                timer_id = invalid_timer_id;
            }

            template<class T>
            void check_request_session(T &msg, session_func_t pass_func, session_func_t unpass_func){
                auto iter = _request_session.find(msg.header.sequence);
                if (iter == _request_session.end())
                    return;

                std::shared_ptr<request_session_t> session = iter->second;
                msg.result == raft_error::success ? ++session->pass_count : ++session->unpass_count;

                if (session->pass_count > session->request_count / 2){
                    cancel_timer(session->timer_id);
                    _request_session.erase(iter);

                    if (pass_func)
                        pass_func(session);
                }
                else if (session->unpass_count >= session->request_count / 2) {
                    cancel_timer(session->timer_id);
                    _request_session.erase(iter);

                    if (unpass_func)
                        unpass_func(session);
                }
            }

            void handle_request_session_timeout(uint64 sequence){
                auto iter = _request_session.find(sequence);
                if (iter == _request_session.end())
                    return;

                shared_ptr<request_session_t> sess = iter->second;

                if (sess->identify == raft_vote_request::identify)
                {
                    //等待投票应答超时,重新设置投票定时器
                    set_timer(_vote_timer, _config.vote_wait_base, std::bind(&node::handle_vote_timer, this));
                }
                else if (sess->identify == raft_execute_request::identify
                    || sess->identify == raft_submit_request::identify
                    || sess->identify == raft_commit_request::identify
                    )
                {
                    raft_execute_response rsp;
                    rsp.header.sequence = sess->sequence;
                    rsp.result = raft_error::timeout;
                    send(rsp, nullptr, sess->conn);
                }
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
                            //NODE_LOG("send type:%s sequence:%llu term:%llu commit:%llu min_uncommit:%llu max_uncommit:%llu", std::string(typeid(msg).name()).substr(7).c_str(), msg.header.sequence, msg.header.term, msg.header.commit, msg.header.min_uncommit, msg.header.max_uncommit);

                        }
                    }
                }

                if (peer)
                {
                    peer->async_sendv(iov, 2);
                    if (msg.identify != raft_heartbeat::identify)
                    {
                        //NODE_LOG("send type:%s sequence:%llu term:%llu commit:%llu min_uncommit:%llu max_uncommit:%llu", std::string(typeid(msg).name()).substr(7).c_str(), msg.header.sequence, msg.header.term, msg.header.commit, msg.header.min_uncommit, msg.header.max_uncommit);
                    }
                }
            }

            void fill_header(raft_header &header){
                header.type = _config.type;
                header.id = _config.id;
                header.leader = _leader;
                header.term = _status.term;
                header.commit = get_commit_id();
                header.min_uncommit = get_min_uncommit_id();
                header.max_uncommit = get_max_uncommit_id();
            }

            void fill_request_header(raft_header &header){
                fill_header(header);
                header.sequence = gen_sequence();
            }

            void fill_response_header(const raft_header &req, raft_header &rsp){
                fill_header(rsp);
                rsp.sequence = req.sequence;
            }

            void save_status(){
                _status_file.save(_status.shine_serial_encode());
            }
        private:
            bool _get_entry_process = false;
            raft_execute_func_t _execute_func = nullptr;
            uint32 _sequence = 0;
            raft_config _config;
            string _config_file_path;
            raft_node_info _self_info;
            raft_node_status _status;
            file _status_file;

            raft_node_entry _commit_entry;
            raft_node_entry _uncommit_entry;
            file _entry_file;

            net::proactor_engine _engine;
            std::shared_ptr<std::thread> _thread;
            int32 _thread_id = 0;
            std::set<connection*> _server_remote_peers;
            std::map<string, connection*> _client_peers;
            connection * _leader_peer = nullptr;
            uint64 _heartbeat_timer = invalid_timer_id;
            uint64 _vote_timer = invalid_timer_id;
            uint64 _get_entry_timer = invalid_timer_id;
            bool _leader = false;

            std::unordered_map<uint64, std::shared_ptr<request_session_t> > _request_session;

            log _log;
            int32 _log_level = log::e_debug;
            int32 _log_output = log::e_console | log::e_file;

            std::pair<socket_t, socket_t> _signal_pipe;
            block_concurrent_queue<std::string> _execute_queue;
            std::shared_ptr<rpc::pipe_server> _rpc_server;
            std::shared_ptr<rpc::pipe_client> _rpc_client;
            std::condition_variable _execute_cv;
            std::recursive_mutex _execute_mutex;
            std::unordered_map<int32, std::shared_ptr<rpc_channel_t> > _execute_cannels;

        };
    }
}
