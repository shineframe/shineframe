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


        struct set_option_t{
            SHINE_GEN_MEMBER_GETSET(uint64, EX, = 0);
            SHINE_GEN_MEMBER_GETSET(uint64, PX, = 0);
            SHINE_GEN_MEMBER_GETSET(bool, NX, = false);
            SHINE_GEN_MEMBER_GETSET(bool, XX, = false);
        };


        class sync_string{
        public:
            sync_string(sync_client_t client)
            {
                set_sync_client(client);
            }
            virtual ~sync_string(){

            }

            int64 APPEND(const string &key, const string &value){
                SYNC_CALL(-1, "APPEND", key, value);

                return std::stoll(SYNC_HEADER_VALUE);
            }

            int64 BITCOUNT(const string &key, int64 begin = 0, int64 end = -1){
                SYNC_CALL(-1, "BITCOUNT", key, begin, end);

                return std::stoll(SYNC_HEADER_VALUE);
            }

            int64 BITFIELD(...){
                return -1;
            }

            template<class ...ARGS>
            int64 BITOP(const string &store, ARGS ... args){
                SYNC_CALL(-1, "BITOP", store, args...);

                return std::stoll(SYNC_HEADER_VALUE);
            }

            int64 BITPOS(const string &key, int64 begin = 0, int64 end = -1){
                SYNC_CALL(-1, "BITPOS", key, begin, end);

                return std::stoll(SYNC_HEADER_VALUE);
            }

            int64 DECR(const string &key){
                SYNC_CALL(-1, "DECR", key);

                return std::stoll(SYNC_HEADER_VALUE);
            }

            int64 DECRBY(const string &key, int64 value){
                SYNC_CALL(-1, "DECRBY", key, value);

                return std::stoll(SYNC_HEADER_VALUE);
            }

            bool GET(const string &key, string &value){
                SYNC_CALL(false, "GET", key);

                result_parse::bulk_string_reply(SYNC_DATA_VALUE, value);
                return !value.empty();
            }

            uint64 GETBIT(const string &key, int64 offset){
                SYNC_CALL(0, "GETBIT", key, offset);

                return std::stoull(SYNC_HEADER_VALUE);
            }

            bool GETRANGE(const string &key, int64 begin, int64 end, string &value){
                SYNC_CALL(false, "GETRANGE", key, begin, end);

                result_parse::bulk_string_reply(SYNC_DATA_VALUE, value);
                return !value.empty();
            }

            bool GETSET(const string &key, const string &new_value, string &old_value){
                SYNC_CALL(false, "GETSET", key, new_value);

                result_parse::bulk_string_reply(SYNC_DATA_VALUE, old_value);
                return !old_value.empty();
            }

            int64 INCR(const string &key){
                SYNC_CALL(-1, "INCR", key);

                return std::stoll(SYNC_HEADER_VALUE);
            }

            int64 INCRBY(const string &key, int64 value){
                SYNC_CALL(-1, "INCRBY", key, value);

                return std::stoll(SYNC_HEADER_VALUE);
            }

            double INCRBYFLOAT(const string &key, double value){
                SYNC_CALL(-1, "INCRBYFLOAT", key, value);

                return std::stod(SYNC_DATA_VALUE.get_clients()->front()->get_value());
            }

            template<class ...ARGS>
            bool MGET(std::vector<string> &arr, ARGS ... args){
                SYNC_CALL(false, "MGET", args...);

                result_parse::array_reply(SYNC_DATA_VALUE, arr);

                return SYNC_HEADER_VALUE != "-1";
            }

            template<class ...ARGS>
            bool MSET(ARGS ... args){
                SYNC_CALL(false, "MSET", args...);

                return true;
            }

            template<class ...ARGS>
            uint64 MSETNX(ARGS ... args){
                SYNC_CALL(0, "MSETNX", args...);

                return std::stoull(SYNC_HEADER_VALUE);
            }

            bool PSETEX(const string &key, uint64 milliseconds, const string &value){
                SYNC_CALL(false, "PSETEX", key, milliseconds, value);

                return SYNC_HEADER_TYPE == e_type_ok;
            }

            bool SET(const string &key, const string &value){
                SYNC_CALL(false, "SET", key, value);

                return SYNC_HEADER_TYPE == e_type_ok;
            }

            bool SET(const string &key, const string &value, const set_option_t &option)
            {
                std::vector<string> args;

                args.push_back("SET");
                args.push_back(key);
                args.push_back(value);

                if (option.get_EX() > 0)
                {
                    args.push_back("EX");
                    args.push_back(option.get_EX());
                }

                if (option.get_PX() > 0)
                {
                    args.push_back("PX");
                    args.push_back(option.get_PX());
                }

                if (option.get_NX() && !option.get_XX())
                {
                    args.push_back("NX");
                }

                if (option.get_XX() && !option.get_NX())
                {
                    args.push_back("XX");
                }

                if (!get_sync_client()->call(request::encode(std::move(args))))
                    return false;

                return SYNC_HEADER_TYPE == e_type_ok;
            }

            uint64 SETBIT(const string &key, int64 offset, bool bit = false){
                SYNC_CALL(0, "SETBIT", key, offset, (bit ? 1 : 0));

                return std::stoull(SYNC_HEADER_VALUE);
            }

            bool SETEX(const string &key, const string &value){
                SYNC_CALL(false, "SETEX", key, value);

                return SYNC_HEADER_TYPE == e_type_ok;
            }

            bool SETNX(const string &key, const string &value){
                SYNC_CALL(false, "SETNX", key, value);

                return SYNC_HEADER_TYPE == e_type_ok;
            }

            uint64 SETRANGE(const string &key, int64 offset, const string &value){
                SYNC_CALL(0, "SETRANGE", key, offset, value);

                return std::stoull(SYNC_HEADER_VALUE);
            }

            uint64 STRLEN(const string &key){
                SYNC_CALL(0, "STRLEN", key);

                return std::stoull(SYNC_HEADER_VALUE);
            }


        private:
            SHINE_GEN_MEMBER_GETSET(sync_client_t, sync_client, = nullptr);
        };

    }
}
