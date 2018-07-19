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

        class sync_sorted_set{
        public:
            sync_sorted_set(sync_client_t client)
            {
                set_sync_client(client);
            }
            virtual ~sync_sorted_set(){

            }

            template<class ...ARGS>
            uint64 ZADD(const string &key, ARGS ... args){
                SYNC_CALL(0, "ZADD", key, args...);

                return std::stoll(SYNC_HEADER_VALUE);
            }

            uint64 ZCARD(const string &key){
                SYNC_CALL(0, "ZCARD", key);

                return std::stoull(SYNC_HEADER_VALUE);
            }

            uint64 ZCOUNT(const string &key){
                SYNC_CALL(0, "ZCOUNT", key, "-inf", "+inf");

                return std::stoull(SYNC_HEADER_VALUE);
            }

            uint64 ZCOUNT(const string &key, double min, double max){
                SYNC_CALL(0, "ZCOUNT", key, min, max);

                return std::stoull(SYNC_HEADER_VALUE);
            }

            double ZINCRBY(const string &key, double increment, const string &member){
                SYNC_CALL(0, "ZINCRBY", key, increment, member);

                return std::stod(SYNC_HEADER_VALUE);
            }

            uint64 ZINTERSTORE(const string &store, const std::vector<string> &keys, const std::vector<double> &weights = {}){

                std::vector<string> args;

                args.push_back("ZINTERSTORE");
                args.push_back(store);
                args.push_back((uint64)keys.size());

                for (auto &key : keys)
                    args.push_back(key);

                if (weights.size() > 0)
                {
                    args.push_back("WEIGHT");
                    for (auto &weight : weights)
                        args.push_back(weight);
                }

                if (!get_sync_client()->call(request::encode(std::move(args)))) 
                    return 0; 

                if (SYNC_HEADER_TYPE == e_type_error)
                    return 0;

                return std::stoull(SYNC_HEADER_VALUE);
            }

            uint64 ZLEXCOUNT(const string &key, const string &min, const string &max){
                SYNC_CALL(0, "ZLEXCOUNT", key, min, max);

                return std::stoull(SYNC_HEADER_VALUE);
            }

            bool ZRANGE(const string &key, int64 start, int64 stop, std::vector<string> &arr){
                SYNC_CALL(false, "ZRANGE", key, start, stop);

                result_parse::array_reply(SYNC_DATA_VALUE, arr);

                return SYNC_HEADER_VALUE != "-1";
            }

            bool ZRANGE(const string &key, int64 start, int64 stop, std::vector<std::pair<string, double> > &arr){
                SYNC_CALL(false, "ZRANGE", key, start, stop, "WITHSCORES");

                result_parse::array_reply(SYNC_DATA_VALUE, arr);

                return SYNC_HEADER_VALUE != "-1";
            }

            bool ZRANGEBYLEX(const string &key, const string &min, const string &max, int64 offset, int64 count, std::vector<string> &arr){
                SYNC_CALL(false, "ZRANGEBYLEX", key, min, max, "LIMIT", offset, count);

                result_parse::array_reply(SYNC_DATA_VALUE, arr);

                return SYNC_HEADER_VALUE != "-1";
            }

            bool ZRANGEBYLEX(const string &key, const string &min, const string &max, int64 offset, int64 count, std::set<string> &set){
                SYNC_CALL(false, "ZRANGEBYLEX", key, min, max, "LIMIT", offset, count);

                result_parse::array_reply(SYNC_DATA_VALUE, set);

                return SYNC_HEADER_VALUE != "-1";
            }

            bool ZRANGEBYSCORE(const string &key, const string &min, const string &max, int64 offset, int64 count, std::vector<string> &arr){
                SYNC_CALL(false, "ZRANGEBYSCORE", key, min, max, "LIMIT", offset, count);

                result_parse::array_reply(SYNC_DATA_VALUE, arr);

                return SYNC_HEADER_VALUE != "-1";
            }

            bool ZRANGEBYSCORE(const string &key, const string &min, const string &max, int64 offset, int64 count, std::vector<std::pair<string, double> > &arr){
                SYNC_CALL(false, "ZRANGEBYSCORE", key, min, max, "LIMIT", offset, count, "WITHSCORES");

                result_parse::array_reply(SYNC_DATA_VALUE, arr);

                return SYNC_HEADER_VALUE != "-1";
            }

            int64 ZRANK(const string &key, const string &member){
                SYNC_CALL(-1, "ZRANK", key, member);

                return std::stoll(SYNC_HEADER_VALUE);
            }

            template<class ...ARGS>
            uint64 ZREM(const string &key, ARGS ... args){
                SYNC_CALL(0, "ZREM", key, args...);

                return std::stoull(SYNC_HEADER_VALUE);
            }

            uint64 ZREMRANGEBYLEX(const string &key, const string &min, const string &max){
                SYNC_CALL(0, "ZREMRANGEBYLEX ", key, min, max);

                return std::stoull(SYNC_HEADER_VALUE);
            }

            uint64 ZREMRANGEBYRANK(const string &key, int64 start, int64 stop){
                SYNC_CALL(0, "ZREMRANGEBYRANK ", key, start, stop);

                return std::stoull(SYNC_HEADER_VALUE);
            }

            uint64 ZREMRANGEBYSCORE(const string &key, const string &min, const string &max){
                SYNC_CALL(0, "ZREMRANGEBYSCORE ", key, min, max);

                return std::stoull(SYNC_HEADER_VALUE);
            }

            uint64 ZREMRANGEBYSCORE(const string &key, double min, double max){
                SYNC_CALL(0, "ZREMRANGEBYSCORE ", key, min, max);

                return std::stoull(SYNC_HEADER_VALUE);
            }

            bool ZREVRANGE(const string &key, int64 start, int64 stop, std::vector<string> &arr){
                SYNC_CALL(false, "ZREVRANGE", key, start, stop);

                result_parse::array_reply(SYNC_DATA_VALUE, arr);

                return SYNC_HEADER_VALUE != "-1";
            }

            bool ZREVRANGE(const string &key, int64 start, int64 stop, std::vector<std::pair<string, double> > &arr){
                SYNC_CALL(false, "ZREVRANGE", key, start, stop, "WITHSCORES");

                result_parse::array_reply(SYNC_DATA_VALUE, arr);

                return SYNC_HEADER_VALUE != "-1";
            }

            bool ZREVRANGEBYLEX(const string &key, const string &min, const string &max, int64 offset, int64 count, std::vector<string> &arr){
                SYNC_CALL(false, "ZREVRANGEBYLEX", key, min, max, "LIMIT", offset, count);

                result_parse::array_reply(SYNC_DATA_VALUE, arr);

                return SYNC_HEADER_VALUE != "-1";
            }

            bool ZREVRANGEBYLEX(const string &key, const string &min, const string &max, int64 offset, int64 count, std::set<string> &set){
                SYNC_CALL(false, "ZREVRANGEBYLEX", key, min, max, "LIMIT", offset, count);

                result_parse::array_reply(SYNC_DATA_VALUE, set);

                return SYNC_HEADER_VALUE != "-1";
            }


            bool ZREVRANGEBYSCORE(const string &key, const string &min, const string &max, int64 offset, int64 count, std::vector<string> &arr){
                SYNC_CALL(false, "ZREVRANGEBYSCORE", key, min, max, "LIMIT", offset, count);

                result_parse::array_reply(SYNC_DATA_VALUE, arr);

                return SYNC_HEADER_VALUE != "-1";
            }

            bool ZREVRANGEBYSCORE(const string &key, const string &min, const string &max, int64 offset, int64 count, std::vector<std::pair<string, double> > &arr){
                SYNC_CALL(false, "ZREVRANGEBYSCORE", key, min, max, "LIMIT", offset, count, "WITHSCORES");

                result_parse::array_reply(SYNC_DATA_VALUE, arr);

                return SYNC_HEADER_VALUE != "-1";
            }

            int64 ZREVRANK(const string &key, const string &member){
                SYNC_CALL(-1, "ZREVRANK", key, member);

                return std::stoll(SYNC_HEADER_VALUE);
            }

            uint64 ZSCAN(const string &key, std::vector<std::pair<string, double> > &members, uint64 offset = 0, uint64 count = 10, const string &match = "*"){
                SYNC_CALL(0, "ZSCAN", key, offset, "COUNT", count, "MATCH", match);
                if (SYNC_CLIENTS_VALUE)
                {
                    uint64 ret = 0;
                    result_parse::scan_reply(SYNC_DATA_VALUE, ret, members);
                    return ret;
                }
                else
                    return 0;
            }

            bool ZSCORE(const string &key, const string &member, double &score){
                SYNC_CALL(false, "ZSCORE", key, member);

                string str;
                if (SYNC_HEADER_VALUE != "-1")
                {
                    result_parse::bulk_string_reply(SYNC_DATA_VALUE, str);
                    score = std::stod(str);
                    return true;
                }

                return false;
            }

            uint64 ZUNIONSTORE(const string &store, const std::vector<string> &keys, const std::vector<double> &weights = {}){

                std::vector<string> args;

                args.push_back("ZUNIONSTORE");
                args.push_back(store);
                args.push_back((uint64)keys.size());

                for (auto &key : keys)
                    args.push_back(key);

                if (weights.size() > 0)
                {
                    args.push_back("WEIGHT");
                    for (auto &weight : weights)
                        args.push_back(weight);
                }

                if (!get_sync_client()->call(request::encode(std::move(args))))
                    return 0;

                if (SYNC_HEADER_TYPE == e_type_error)
                    return 0;

                return std::stoull(SYNC_HEADER_VALUE);
            }

        private:
            SHINE_GEN_MEMBER_GETSET(sync_client_t, sync_client, = nullptr);
        };

    }
}
