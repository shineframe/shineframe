#pragma once

#include <iostream>
#include "../util/json.hpp"
#include "../shine_serial/shine_serial.hpp"

#if (defined SHINE_OS_WINDOWS)
#else
#endif

struct raft_node_status{
    shine::uint64 term = 0;
    shine::uint64 commit = 0;
    shine::uint64 uncommit = 0;
    SHINE_SERIAL(raft_node_status, term, commit, uncommit);
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
    shine::string id;
    shine::string ip;
    shine::uint16 port;

    SHINE_JSON_MODEL(raft_node_info, id, ip, port);
};

struct raft_config{
    shine::string id;
    shine::uint32 heartbeat = 2000;
    shine::uint32 heartbeat_timeout_count = 3;
    shine::uint32 vote_wait_base = 1000;
    shine::string status_file = "status.log";
    shine::string entry_file = "entry.log";
    std::map<shine::string, raft_node_info> nodes;

    SHINE_JSON_MODEL(raft_config, id, heartbeat, heartbeat_timeout_count, vote_wait_base, status_file, entry_file, nodes);
};

