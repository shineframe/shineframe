#pragma once

#include <iostream>
#include "../util/json.hpp"
#include "../shine_serial/shine_serial.hpp"

#if (defined SHINE_OS_WINDOWS)
#else
#endif

struct raft_node_status{
    shine::uint64 term = 0;
    SHINE_SERIAL(raft_node_status, term);
};

struct raft_entry{
    shine::uint64 no = 0;
    shine::string data;
    SHINE_SERIAL(raft_entry, no, data);
};

struct raft_node_entry{
    std::deque<raft_entry> entrys;
    SHINE_SERIAL(raft_node_entry, entrys);
};

struct raft_node_info{
    shine::uint16 type;
    shine::uint16 id;
    shine::string ip;
    shine::uint16 port;

    SHINE_JSON_MODEL(raft_node_info, type, id, ip, port);
};

struct raft_config{
    shine::uint16 type = 0;
    shine::uint16 id = 0;
    shine::uint32 heartbeat = 2000;
    shine::uint32 heartbeat_timeout_count = 3;
    shine::uint32 vote_wait_base = 1000;
    shine::uint32 submit_wait_base = 2000;
    shine::uint32 commit_wait_base = 2000;
    shine::uint32 get_entry_wait_base = 5000;
    shine::string status_file = "status.log";
    shine::string entry_file = "entry.log";
    std::map<shine::uint16, raft_node_info> nodes;

    SHINE_JSON_MODEL(raft_config, type, id, heartbeat, heartbeat_timeout_count, vote_wait_base, submit_wait_base, commit_wait_base, get_entry_wait_base, status_file, entry_file, nodes);
};

