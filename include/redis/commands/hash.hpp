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

        class sync_hash{
        public:
            sync_hash(sync_client_t client)
            {
                set_sync_client(client);
            }
            virtual ~sync_hash(){

            }

            template<class ...ARGS>
            uint64 HDEL(const string &key, ARGS ... args){
                SYNC_CALL(false, "HDEL", key, args...);

                return std::stoull(SYNC_HEADER_VALUE);
            }

            bool HEXISTS(const string &key, const string &field){
                SYNC_CALL(false, "HEXISTS", key, field);

                return SYNC_HEADER_VALUE == "1";
            }

            bool HGET(const string &key, const string &field, string &value){
                SYNC_CALL(false, "HGET", key, field);

                result_parse::bulk_string_reply(SYNC_DATA_VALUE, value);
                return SYNC_HEADER_VALUE != "-1";
            }

            bool HGETALL(const string &key, std::map<string, string> &map){
                SYNC_CALL(false, "HGETALL", key);

                result_parse::array_reply(SYNC_DATA_VALUE, map);
                return SYNC_HEADER_VALUE != "-1";
            }

            int64 HINCRBY(const string &key, const string &field, int64 value){
                SYNC_CALL(-1, "HINCRBY", key, field, value);

                return std::stoll(SYNC_HEADER_VALUE);
            }

            double HINCRBYFLOAT(const string &key, const string &field, double value){
                SYNC_CALL(-1, "HINCRBYFLOAT", key, field, value);

                return std::stod(SYNC_DATA_VALUE.get_clients()->front()->get_value());
            }

            bool HKEYS(const string &key, std::set<string> &set){
                SYNC_CALL(false, "HKEYS", key);

                result_parse::array_reply(SYNC_DATA_VALUE, set);
                return SYNC_HEADER_VALUE != "-1";
            }

            uint64 HLEN(const string &key){
                SYNC_CALL(0, "HLEN", key);

                return std::stoull(SYNC_HEADER_VALUE);
            }

            template<class ...ARGS>
            bool HMGET(const string &key, std::vector<string> &arr, ARGS ... args){
                SYNC_CALL(false, "HMGET", key, args...);

                result_parse::array_reply(SYNC_DATA_VALUE, arr);

                return SYNC_HEADER_VALUE != "-1";
            }

            template<class ...ARGS>
            bool HMSET(const string &key, std::vector<string> &arr, ARGS ... args){
                SYNC_CALL(false, "HMSET", key, args...);

                return SYNC_HEADER_TYPE == e_type_ok;
            }

            uint64 HSCAN(const string &key, std::map<string, string> &map, uint64 offset = 0, uint64 count = 10, const string &match = "*"){
                SYNC_CALL(0, "HSCAN", key, offset, "COUNT", count, "MATCH", match);
                if (SYNC_CLIENTS_VALUE)
                {
                    uint64 ret = 0;
                    result_parse::scan_reply(SYNC_DATA_VALUE, ret, map);
                    return ret;
                }
                else
                    return 0;
            }

            bool HSET(const string &key, const string &field, const string &value){
                SYNC_CALL(false, "HSET", key, field, value);

                return SYNC_HEADER_TYPE != e_type_error;
            }

            bool HSETNX(const string &key, const string &field, const string &value){
                SYNC_CALL(false, "HSETNX", key, field, value);

                return SYNC_HEADER_VALUE == "1";
            }

            uint64 HSTRLEN(const string &key, const string &field){
                SYNC_CALL(0, "HSTRLEN", key, field);

                return std::stoull(SYNC_HEADER_VALUE);
            }

            bool HVALS(const string &key, std::set<string> &set){
                SYNC_CALL(false, "HVALS", key);

                result_parse::array_reply(SYNC_DATA_VALUE, set);
                return SYNC_HEADER_VALUE != "-1";
            }

        private:
            SHINE_GEN_MEMBER_GETSET(sync_client_t, sync_client, = nullptr);
        };

    }
}
