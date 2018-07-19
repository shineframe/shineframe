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

        class sync_geo{
        public:
            sync_geo(sync_client_t client)
            {
                set_sync_client(client);
            }
            virtual ~sync_geo(){

            }

            template<class ...ARGS>
            uint64 GEOADD(const string &key, ARGS ... args){
                SYNC_CALL(0, "GEOADD", key, args...);

                return std::stoull(SYNC_HEADER_VALUE);
            }

            bool GEODIST(double &value, const string &key, const string &member1, const string &member2,  const string &unit = "m"){
                SYNC_CALL(false, "GEODIST", key, member1, member2, unit);

                if (SYNC_HEADER_VALUE != "")
                {
                    value = std::stod(SYNC_HEADER_VALUE);
                    return true;
                }

                return false;
            }


            template<class ...ARGS>
            bool GEOHASH(std::vector<string> &arr, const string &key, ARGS ... args){
                SYNC_CALL(false, "GEOHASH", key, args...);

                result_parse::array_reply(SYNC_DATA_VALUE, arr);

                return true;
            }

            bool GEOPOS(const string &key, const std::vector<string> &members, std::vector<std::pair<string, string>> &arr){
                std::vector<string> args;

                args.push_back("GEOPOS");
                args.push_back(key);

                for (auto &member : members)
                    args.push_back(member);

                if (!get_sync_client()->call(request::encode(std::move(args))))
                    return false;

                result_parse::GEOPOS_reply(SYNC_DATA_VALUE, arr);

                return true;
            }

            bool GEORADIUS(const string &key, const string&longitude, const string &latitude, uint64 radius, uint64 count, std::vector<geo_info_t> &arr){

                SYNC_CALL(false, "GEORADIUS", key, longitude, latitude, radius, "m", "WITHCOORD", "WITHDIST", "COUNT", count);

                result_parse::GEORADIUS_reply(SYNC_DATA_VALUE, arr);

                return true;
            }

            bool GEORADIUSBYMEMBER(const string &key, const string&member, uint64 radius, uint64 count, std::vector<geo_info_t> &arr){

                SYNC_CALL(false, "GEORADIUSBYMEMBER", key, member, radius, "m", "WITHCOORD", "WITHDIST", "COUNT", count);

                result_parse::GEORADIUS_reply(SYNC_DATA_VALUE, arr);

                return true;
            }


        private:
            SHINE_GEN_MEMBER_GETSET(sync_client_t, sync_client, = nullptr);
        };
    }
}
