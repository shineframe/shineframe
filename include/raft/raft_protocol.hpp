#pragma once

#include <iostream>
#include "../util/json.hpp"
#include "../shine_serial/shine_serial.hpp"
#include "raft_config.hpp"

#if (defined SHINE_OS_WINDOWS)
#else
#endif

using namespace shine;

struct raft_header{
    shine::string id;
    shine::uint64 sequence = 0;
    shine::Bool leader = false;
    shine::uint64 term = 0;
    shine::uint64 commit = 0;
    shine::uint64 uncommit = 0;
    SHINE_SERIAL(raft_header, id, sequence, leader, term, commit, uncommit);
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
