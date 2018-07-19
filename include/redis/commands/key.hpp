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

        struct sort_option_t{
            SHINE_GEN_MEMBER_GETSET(bool, IS_ALPHA, = false);
            SHINE_GEN_MEMBER_GETSET(bool, DESC, = false);
            SHINE_GEN_MEMBER_GETSET(int64, LIMIT_OFFSET, = 0);
            SHINE_GEN_MEMBER_GETSET(int64, LIMIT_COUNT, = -1);
            SHINE_GEN_MEMBER_GETSET(string, BY);
            SHINE_GEN_MEMBER_GETSET(string, GET1);
            SHINE_GEN_MEMBER_GETSET(string, GET2);
            SHINE_GEN_MEMBER_GETSET(string, STORE);
        };

        class sync_key{
        public:
            sync_key(sync_client_t client)
            {
                set_sync_client(client);
            }
            virtual ~sync_key(){

            }

            bool DEL(const string &key){
                SYNC_CALL(false, "DEL", key);

                return SYNC_HEADER_VALUE == "1";
            }

            bool DUMP(const string &key, string &value){
                SYNC_CALL(false, "DUMP", key);

                if (!SYNC_CLIENTS_VALUE)
                    return false;
                else
                {
                    value = SYNC_CLIENTS_VALUE->front()->get_value();
                    return true;
                }
            }

            bool EXISTS(const string &key){
                SYNC_CALL(false, "EXISTS", key);

                return SYNC_HEADER_VALUE == "1";
            }

            bool EXPIRE(const string &key, uint64 seconds){
                SYNC_CALL(false, "EXPIRE", key, seconds);

                return SYNC_HEADER_VALUE == "1";
            }

            bool EXPIREAT(const string &key, uint64 seconds_timestamp){
                SYNC_CALL(false, "EXPIREAT", key, seconds_timestamp);

                return SYNC_HEADER_VALUE == "1";
            }

            bool KEYS(const string &pattern, std::vector<string> &keys){
                SYNC_CALL(false, "KEYS", pattern);

                result_parse::array_reply(SYNC_DATA_VALUE, keys);
                return true;
            }

            bool MIGRATE(){
                return false;
            }

            bool MOVE(const string &key, uint8 database){
                SYNC_CALL(false, "MOVE", key, database);

                return SYNC_HEADER_VALUE == "1";
            }

            bool OBJECT(){
                return false;
            }

            bool PERSIST(const string &key){
                SYNC_CALL(false, "PERSIST", key);

                return SYNC_HEADER_VALUE == "1";
            }

            bool PEXPIRE(const string &key, uint64 milliseconds){
                SYNC_CALL(false, "PEXPIRE", key, milliseconds);

                return SYNC_HEADER_VALUE == "1";
            }

            bool PEXPIREAT(const string &key, uint64 milliseconds_timestamp){
                SYNC_CALL(false, "PEXPIREAT", key, milliseconds_timestamp);

                return SYNC_HEADER_VALUE == "1";
            }

            int64 PTTL(const string &key){
                SYNC_CALL(-1, "PTTL", key);

                return std::stoll(SYNC_HEADER_VALUE);
            }

            int64 TTL(const string &key){
                SYNC_CALL(-1, "TTL", key);

                return std::stoll(SYNC_HEADER_VALUE);
            }

            bool RANDOMKEY(string &key){
                SYNC_CALL(false, "RANDOMKEY");

                if (SYNC_CLIENTS_VALUE)
                {
                    result_parse::bulk_string_reply(SYNC_DATA_VALUE, key);
                    return true;
                }
                else
                    return false;
            }

            bool RENAME(const string &key, const string &new_key){
                SYNC_CALL(false, "RENAME", key, new_key);

                return SYNC_HEADER_TYPE == e_type_ok;
            }

            bool RENAMENX(const string &key, const string &new_key){
                SYNC_CALL(false, "RENAMENX", key, new_key);

                return SYNC_HEADER_VALUE == "1";
            }

            bool RESTORE(){
                return false;
            }

            bool TYPE(const string &key, string &type){
                SYNC_CALL(false, "TYPE", key);

                type = SYNC_HEADER_VALUE;
                return type == "1";
            }

            bool WAIT(){
                return false;
            }

            uint64 SCAN(std::vector<string> &keys, uint64 offset = 0, uint64 count = 10, const string &match = "*"){
                SYNC_CALL(0, "SCAN", offset, "COUNT", count, "MATCH", match);
                if (SYNC_CLIENTS_VALUE)
                {
                    uint64 ret = 0;
                    result_parse::scan_reply(SYNC_DATA_VALUE, ret, keys);
                    return ret;
                }
                else
                    return 0;
            }

            bool SORT(const string &key, const sort_option_t &option, std::vector<string> &values)
            {
                std::vector<string> args;

                args.push_back("SORT");
                args.push_back(key);

                if (option.get_IS_ALPHA())
                    args.push_back("ALPHA");

                if (!option.get_BY().empty())
                {
                    args.push_back("BY");
                    args.push_back(option.get_BY());
                }

                if (option.get_DESC())
                    args.push_back("DESC");

                if (option.get_LIMIT_OFFSET() != 0)
                {
                    args.push_back("LIMIT");
                    args.push_back(option.get_LIMIT_OFFSET());
                    args.push_back(option.get_LIMIT_COUNT());
                }

                if (!option.get_GET1().empty())
                {
                    args.push_back("GET");
                    args.push_back(option.get_GET1());
                }

                if (!option.get_GET2().empty())
                {
                    args.push_back("GET");
                    args.push_back(option.get_GET2());
                }

                if (!option.get_STORE().empty())
                {
                    args.push_back("STORE");
                    args.push_back(option.get_STORE());
                }

                if (!get_sync_client()->call(request::encode(std::move(args))))
                    return false;

                result_parse::array_reply(SYNC_DATA_VALUE, values);
                return true;
            }

        private:
            SHINE_GEN_MEMBER_GETSET(sync_client_t, sync_client, = nullptr);
        };
    }
}
