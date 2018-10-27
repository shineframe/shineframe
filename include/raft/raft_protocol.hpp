#pragma once

#include <iostream>
#include "../util/json.hpp"
#include "../shine_serial/shine_serial.hpp"
#include "raft_config.hpp"

#if (defined SHINE_OS_WINDOWS)
#else
#endif

using namespace shine;

struct raft_error{
    static const shine::int32 success = 0;
    static const shine::int32 disagree = 1;
    static const shine::int32 unable_execute = 2;
    static const shine::int32 decode_failed = 3;
    static const shine::int32 timeout = 4;
    static const shine::int32 submit_failed = 5;
    static const shine::int32 commit_failed = 6;
};

struct raft_header{
    shine::uint16 type = 0;
    shine::uint16 id = 0;
    shine::uint64 sequence = 0;
    shine::Bool leader = false;
    shine::uint64 term = 0;
    shine::uint64 commit = 0;
    shine::uint64 min_uncommit = 0;
    shine::uint64 max_uncommit = 0;
    SHINE_SERIAL(raft_header, type, id, sequence, leader, term, commit, min_uncommit, max_uncommit);
};

struct raft_heartbeat{
    static const shine::size_t identify = 0;
    raft_header header;
    SHINE_SERIAL(raft_heartbeat, header);
};

struct raft_vote_request{
    static const shine::size_t identify = 1;
    raft_header header;
    SHINE_SERIAL(raft_vote_request, header);
};

struct raft_vote_response{
    static const shine::size_t identify = 2;
    raft_header header;
    shine::int32 result = 0;
    SHINE_SERIAL(raft_vote_response, header, result);
};

struct raft_submit_request{
    static const shine::size_t identify = 3;
    raft_header header;
    raft_entry entry;
    SHINE_SERIAL(raft_submit_request, header, entry);
};

struct raft_submit_response{
    static const shine::size_t identify = 4;
    raft_header header;
    shine::uint64 no;
    shine::int32 result = 0;
    SHINE_SERIAL(raft_submit_response, header, no, result);
};

struct raft_commit_request{
    static const shine::size_t identify = 5;
    raft_header header;
    shine::uint64 no;
    SHINE_SERIAL(raft_commit_request, header, no);
};

struct raft_commit_response{
    static const shine::size_t identify = 6;
    raft_header header;
    shine::uint64 no;
    shine::int32 result = 0;
    SHINE_SERIAL(raft_commit_response, header, no, result);
};

struct raft_get_entry_request{
    static const shine::size_t identify = 7;
    raft_header header;
    SHINE_SERIAL(raft_get_entry_request, header);
};

struct raft_get_entry_response{
    static const shine::size_t identify = 8;
    raft_header header;
    shine::int32 result = 0;
    std::vector<raft_entry> entrys;
    SHINE_SERIAL(raft_get_entry_response, header, result, entrys);
};

struct raft_execute_request{
    static const shine::size_t identify = 101;
    raft_header header;
    bool persistent = true;
    std::string data;
    SHINE_SERIAL(raft_execute_request, header, persistent, data);
};

struct raft_execute_response{
    static const shine::size_t identify = 102;
    raft_header header;
    shine::int32 result = raft_error::success;
    std::string data;
    SHINE_SERIAL(raft_execute_response, header, result, data);
};
