#include <iostream>
#include "raft/raft.hpp"

//简单kv内存数据库实现示例

//命令协议定义
struct kv_header{
    int command;
    SHINE_SERIAL(kv_header, command);
};

struct kv_get_req{
    static const int identify = 0;
    std::string key;
    SHINE_SERIAL(kv_get_req, key);
};

struct kv_get_rsp{
    static const int identify = 1;
    int result;
    std::string value;
    SHINE_SERIAL(kv_get_rsp, result, value);
};

struct kv_set_req{
    static const int identify = 2;
    std::string key;
    std::string value;
    SHINE_SERIAL(kv_set_req, key, value);
};

struct kv_set_rsp{
    static const int identify = 3;
    int result;
    SHINE_SERIAL(kv_set_rsp, result);
};

struct kv_del_req{
    static const int identify = 4;
    std::string key;
    SHINE_SERIAL(kv_del_req, key);
};

struct kv_del_rsp{
    static const int identify = 5;
    int result;
    SHINE_SERIAL(kv_del_rsp, result);
};

namespace kv{
    template<class T>
    void request_encode(const T &msg, std::string &output){
        kv_header head;
        head.command = msg.identify;
        output.assign(std::move(head.shine_serial_encode()));
        output.append(std::move(msg.shine_serial_encode()));
    }

    class business{
    public:
        typedef std::function<void(const char*data, std::size_t len, std::string *output)> handle_func_t;
        business(){
            _handle_map.emplace(0, std::move(std::bind(&business::handle_get_req, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
            _handle_map.emplace(2, std::move(std::bind(&business::handle_set_req, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
            _handle_map.emplace(4, std::move(std::bind(&business::handle_del_req, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
        }

        void worker_func(const std::string &input, std::string *output/*output != nullptr时则需要返回数据*/)
        {
            kv_header head_req;
            std::size_t cost_len = 0;

            if (!head_req.shine_serial_decode(input.data(), input.size(), cost_len))
                return;

            auto iter = _handle_map.find(head_req.command);
            if (iter != _handle_map.end())
                iter->second(input.data() + cost_len, input.size() - cost_len, output);
        }

    private:
        void handle_get_req(const char *data, std::size_t len, std::string *output){
            if (output == nullptr)
                return;
            kv_get_req req;
            kv_get_rsp rsp;

            if (!req.shine_serial_decode(data, len))
                return;

            auto iter = _data.find(req.key);
            if (iter != _data.end())
            {
                rsp.result = 0;
                rsp.value = iter->second;
            }
            else{
                rsp.result = -1;
            }

            output->assign(std::move(rsp.shine_serial_encode()));
        }

        void handle_set_req(const char *data, std::size_t len, std::string *output){
            kv_set_req req;
            if (!req.shine_serial_decode(data, len))
                return;

            _data.emplace(std::move(req.key), std::move(req.value));

            if (output == nullptr)
                return;

            kv_set_rsp rsp;
            rsp.result = 0;
            output->assign(std::move(rsp.shine_serial_encode()));
        }

        void handle_del_req(const char *data, std::size_t len, std::string *output){
            kv_del_req req;
            if (!req.shine_serial_decode(data, len))
                return;

            bool exist = false;
            auto iter = _data.find(req.key);
            if (iter != _data.end())
            {
                exist = true;
                _data.erase(iter);
            }

            if (output == nullptr)
                return;

            kv_del_rsp rsp;
            rsp.result = exist ? 0 : -1;
            output->assign(std::move(rsp.shine_serial_encode()));
        }

    private:
        std::unordered_map<std::string, std::string> _data;
        std::unordered_map<int, handle_func_t> _handle_map;
    };

    class proxy{
    public:
        template<class TREQ, class TRSP>
        bool execute(const TREQ &req, TRSP &rsp, bool persistent = true){
            std::string input;
            std::string output;

            request_encode(req, input);
            _raft_node->execute(input, output, persistent);
            if (!rsp.shine_serial_decode(output))
                return false;

            return true;
        }

        bool get(const std::string &key, std::string &value)
        {
            kv_get_req req;
            kv_get_rsp rsp;
            req.key = key;

            if (!execute(req, rsp, false))
                return false;

            if (rsp.result == 0)
                value.assign(std::move(rsp.value));
            return rsp.result == 0;
        }

        bool set(const std::string &key, const std::string &value)
        {
            kv_set_req req;
            kv_set_rsp rsp;
            req.key = key;
            req.value = value;

            if (!execute(req, rsp))
                return false;

            return rsp.result == 0;
        }
        
        bool del(const std::string &key)
        {
            kv_del_req req;
            kv_del_rsp rsp;
            req.key = key;

            if (!execute(req, rsp))
                return false;

            return true;
        }

    protected:
        std::shared_ptr<shine::raft::node> _raft_node;
    };
}

class warp : public kv::proxy, public kv::business{
public:
    warp(const char *raft_cfg){
        _raft_node = std::make_shared<shine::raft::node>(raft_cfg, std::bind(&kv::business::worker_func, this, std::placeholders::_1, std::placeholders::_2));
    }
private:
};

int main(){
    warp kv_example("raft.cfg");
    std::this_thread::sleep_for(std::chrono::seconds(2));

    for (;;)
    {
        int action = 0;
        std::string key, value;
        bool rc = true;
        std::cout << "1:get  2:set  3:del choose:";
        std::cin >> action;
        if (action == 1)
        { 
            std::cout << "input key :";
            std::cin >> key;
            rc = kv_example.get(key, value);
            std::cout << "result:" << (rc ? "success" : "failed") << "  value:" << value << std::endl;
        }
        else if (action == 2)
        {
            std::cout << "input key value :";
            std::cin >> key >> value;
            rc = kv_example.set(key, value);
            std::cout << "result:" << (rc ? "success" : "failed") << std::endl;
        }
        else if (action == 3)
        {
            std::cout << "input key :";
            std::cin >> key;
            rc = kv_example.del(key);
            std::cout << "result:" << (rc ? "success" : "failed") << std::endl;
        }
    }
    return 0;
}
