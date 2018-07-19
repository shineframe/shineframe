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

        class sync_pub_sub{
        public:
            sync_pub_sub(sync_client_t client)
            {
                set_sync_client(client);
            }
            virtual ~sync_pub_sub(){

            }

            template<class ...ARGS>
            bool PSUBSCRIBE(ARGS ... args){
                SYNC_CALL(false, "PSUBSCRIBE", args...);

                return true;
            }

            uint64 PUBLISH(const string &channel, const string &message){
                SYNC_CALL(0, "PUBLISH", channel, message);

                return std::stoull(SYNC_HEADER_VALUE);
            }

            template<class ...ARGS>
            bool PUBSUB(ARGS ... args){

                return false;
            }

            template<class ...ARGS>
            bool PUNSUBSCRIBE(ARGS ... args){
                SYNC_CALL(false, "PUNSUBSCRIBE", args...);

                return true;
            }

            template<class ...ARGS>
            bool SUBSCRIBE(ARGS ... args){
                SYNC_CALL(false, "SUBSCRIBE", args...);

                return true;
            }

            template<class ...ARGS>
            bool UNSUBSCRIBE(ARGS ... args){
                SYNC_CALL(false, "UNSUBSCRIBE", args...);

                return true;
            }



        private:
            SHINE_GEN_MEMBER_GETSET(sync_client_t, sync_client, = nullptr);
        };
    }
}
