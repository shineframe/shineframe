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

        class sync_hyper_log_log{
        public:
            sync_hyper_log_log(sync_client_t client)
            {
                set_sync_client(client);
            }
            virtual ~sync_hyper_log_log(){

            }

            template<class ...ARGS>
            uint64 PFADD(const string &key, ARGS ... args){
                SYNC_CALL(0, "PFADD", key, args...);

                return std::stoull(SYNC_HEADER_VALUE);
            }

            template<class ...ARGS>
            uint64 PFCOUNT(const string &key, ARGS ... args){
                SYNC_CALL(0, "PFCOUNT", key, args...);

                return std::stoull(SYNC_HEADER_VALUE);
            }

            template<class ...ARGS>
            bool PFMERGE(const string &destkey, ARGS ... args){
                SYNC_CALL(false, "PFMERGE", destkey, args...);

                return SYNC_HEADER_TYPE == e_type_ok;
            }

        private:
            SHINE_GEN_MEMBER_GETSET(sync_client_t, sync_client, = nullptr);
        };
    }
}
