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
#include "../redis_client.hpp"

using namespace std;

namespace shine
{
    namespace redis
    {
        typedef sync_client* sync_client_t;
        typedef async_client* async_client_t;

        class sync_set{
        public:
            sync_set(sync_client_t client)
            {
                set_sync_client(client);
            }
            virtual ~sync_set(){

            }

            template<class ...ARGS>
            int64 SADD(const string &key, ARGS ... args){
                SYNC_CALL(-1, "SADD", key, args...);

                return std::stoll(SYNC_HEADER_VALUE);
            }

            uint64 SCARD(const string &key){
                SYNC_CALL(0, "SCARD", key);

                return std::stoull(SYNC_HEADER_VALUE);
            }

            template<class ...ARGS>
            bool SDIFF(std::set<string> &set, ARGS ... args){
                SYNC_CALL(false, "SDIFF", args...);

                result_parse::array_reply(SYNC_DATA_VALUE, set);
                return true;
            }

            template<class ...ARGS>
            int64 SDIFFSTORE(const string &store, ARGS ... args){
                SYNC_CALL(-1, "SDIFFSTORE", store, args...);

                return std::stoll(SYNC_HEADER_VALUE);
            }

            template<class ...ARGS>
            bool SINTER(std::set<string> &set, ARGS ... args){
                SYNC_CALL(false, "SINTER", args...);

                result_parse::array_reply(SYNC_DATA_VALUE, set);
                return true;
            }

            template<class ...ARGS>
            int64 SINTERSTORE(const string &store, ARGS ... args){
                SYNC_CALL(-1, "SINTERSTORE", store, args...);

                return std::stoll(SYNC_HEADER_VALUE);
            }

            bool SISMEMBER(const string &key, const string &member){
                SYNC_CALL(false, "SISMEMBER", key, member);

                return SYNC_HEADER_VALUE == "1";
            }

            bool SMEMBERS(const string &key, std::set<string> &set){
                SYNC_CALL(false, "SMEMBERS", key);

                result_parse::array_reply(SYNC_DATA_VALUE, set);
                return true;
            }

            bool SMOVE(const string &from, const string &to, const string &member){
                SYNC_CALL(false, "SMOVE", from, to, member);

                return SYNC_HEADER_VALUE == "1";
            }

            bool SPOP(const string &key, uint64 count, std::set<string> &set){
                SYNC_CALL(false, "SPOP", key, count);

                result_parse::array_reply(SYNC_DATA_VALUE, set);
                return set.size() > 0;
            }

            bool SRANDMEMBER(const string &key, uint64 count, std::set<string> &set){
                SYNC_CALL(false, "SRANDMEMBER", key, count);

                result_parse::array_reply(SYNC_DATA_VALUE, set);
                return set.size() > 0;
            }

            template<class ...ARGS>
            uint64 SREM(const string &key, ARGS ... args){
                SYNC_CALL(0, "SREM", key, args...);

                return std::stoull(SYNC_HEADER_VALUE);
            }

            uint64 SSCAN(const string &key, std::set<string> &members, uint64 offset = 0, uint64 count = 10, const string &match = "*"){
                SYNC_CALL(0, "SSCAN", key, offset, "COUNT", count, "MATCH", match);
                if (SYNC_CLIENTS_VALUE)
                {
                    uint64 ret = 0;
                    result_parse::scan_reply(SYNC_DATA_VALUE, ret, members);
                    return ret;
                }
                else
                    return 0;
            }

            template<class ...ARGS>
            bool SUNION(std::set<string> &set, ARGS ... args){
                SYNC_CALL(false, "SUNION", args...);

                result_parse::array_reply(SYNC_DATA_VALUE, set);
                return true;
            }

            template<class ...ARGS>
            int64 SUNIONSTORE(const string &store, ARGS ... args){
                SYNC_CALL(-1, "SUNIONSTORE", store, args...);

                return std::stoll(SYNC_HEADER_VALUE);
            }


        private:
            SHINE_GEN_MEMBER_GETSET(sync_client_t, sync_client, = nullptr);
        };

    }
}
