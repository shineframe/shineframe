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

        class sync_list{
        public:
            sync_list(sync_client_t client)
            {
                set_sync_client(client);
            }
            virtual ~sync_list(){

            }

            template<class ...ARGS>
            bool BLPOP(uint64 timeout_seconds, string &list_key, string &value, ARGS ... args){
                SYNC_CALL(false, "BLPOP", args..., timeout_seconds);

                std::vector<string> arr;
                result_parse::array_reply(SYNC_DATA_VALUE, arr);
                if (arr.size() == 2)
                {
                    list_key = std::move(arr[0]);
                    value = std::move(arr[1]);
                    return true;
                }

                return false;
            }

            template<class ...ARGS>
            bool BRPOP(uint64 timeout_seconds, string &list_key, string &value, ARGS ... args){
                SYNC_CALL(false, "BRPOP", args..., timeout_seconds);

                std::vector<string> arr;
                result_parse::array_reply(SYNC_DATA_VALUE, arr);
                if (arr.size() == 2)
                {
                    list_key = std::move(arr[0]);
                    value = std::move(arr[1]);
                    return true;
                }

                return false;
            }

            bool BRPOPLPUSH(const string &from, const string &to, uint64 timeout_seconds, string &value){
                SYNC_CALL(false, "BRPOPLPUSH", from, to, timeout_seconds);

                value.clear();
                result_parse::bulk_string_reply(SYNC_DATA_VALUE, value);
                return !value.empty();
            }

            bool LINDEX(const string &key, int64 index, string &value){
                SYNC_CALL(false, "LINDEX", key, index);

                value.clear();
                result_parse::bulk_string_reply(SYNC_DATA_VALUE, value);
                return !value.empty();
            }

            int64 LINSERT(const string &key, bool before, const string &pivot, const string &value){
                const string str = before ? "BEFORE" : "AFTER";
                SYNC_CALL(-1, "LINSERT", key, str, pivot, value);

                return std::stoll(SYNC_HEADER_VALUE);
            }

            uint64 LLEN(const string &key){
                SYNC_CALL(0, "LLEN", key);

                return std::stoll(SYNC_HEADER_VALUE);
            }

            bool LPOP(const string &key, string &value){
                SYNC_CALL(false, "LPOP", key);

                result_parse::bulk_string_reply(SYNC_DATA_VALUE, value);

                return true;
            }

            template<class ...ARGS>
            uint64 LPUSH(const string &key, ARGS ... args){
                SYNC_CALL(0, "LPUSH", key, args...);

                return std::stoull(SYNC_HEADER_VALUE);
            }

            uint64 LPUSHX(const string &key, const string &value){
                SYNC_CALL(0, "LPUSHX", key, value);

                return std::stoull(SYNC_HEADER_VALUE);
            }

            bool LRANGE(const string &key, int64 start, int64 stop, std::vector<string> &arr){
                SYNC_CALL(false, "LRANGE", key, start, stop);

                result_parse::array_reply(SYNC_DATA_VALUE, arr);

                return SYNC_HEADER_VALUE != "-1";
            }

            uint64 LREM(const string &key, int64 count, const string &value){
                SYNC_CALL(0, "LREM", key, count, value);

                return std::stoull(SYNC_HEADER_VALUE);
            }

            bool LSET(const string &key, int64 index, const string &value){
                SYNC_CALL(false, "LSET", key, index, value);

                return SYNC_HEADER_TYPE == e_type_ok;
            }

            bool LTRIM(const string &key, int64 start, int64 stop){
                SYNC_CALL(false, "LTRIM", key, start, stop);

                return SYNC_HEADER_TYPE == e_type_ok;
            }

            bool RPOP(const string &key, string &value){
                SYNC_CALL(false, "RPOP", key);

                result_parse::bulk_string_reply(SYNC_DATA_VALUE, value);

                return true;
            }

            bool RPOPLPUSH(const string &from, const string &to, string &value){
                SYNC_CALL(false, "RPOPLPUSH", from, to);

                value.clear();
                result_parse::bulk_string_reply(SYNC_DATA_VALUE, value);
                return !value.empty();
            }

            template<class ...ARGS>
            uint64 RPUSH(const string &key, ARGS ... args){
                SYNC_CALL(0, "RPUSH", key, args...);

                return std::stoull(SYNC_HEADER_VALUE);
            }

            uint64 RPUSHX(const string &key, const string &value){
                SYNC_CALL(0, "RPUSHX", key, value);

                return std::stoull(SYNC_HEADER_VALUE);
            }


        private:
            SHINE_GEN_MEMBER_GETSET(sync_client_t, sync_client, = nullptr);
        };

    }
}
