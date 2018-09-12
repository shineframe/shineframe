#include <iostream>
#include "filesystem/filesystem.hpp"
#include "raft/raft.hpp"

void init_config(int id, shine::raft::node &node){
    raft_config config;

    shine::string path = "raft.cfg";

    shine::file f;
    shine::string buf;

    if (f.open(path, false)){
        f.readall(buf);
        config.json_decode(buf);
        f.close();
    }
    else
    {
        for (int i = 1; i <= 3; i++)
        {
            raft_node_info node;
            node.id = i;

            node.ip = "127.0.0.1";
            node.port = 5000 + i;

            config.nodes.insert(std::make_pair(node.id, node));
        }

        buf = config.json_encode();
        f.open(path, true);
        f.save(buf);
        f.close();
    }

    node._config = config;
    for (auto iter : config.nodes)
    {
        if (iter.first == shine::string(id))
        {
            node._self_info = iter.second;
            break;
        }
    }
}

int main(){

    shine::raft::node node;
//     init_config(1, node);

    node.run("raft.cfg");
    for (;;)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return 0;
}
