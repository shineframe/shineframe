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

        class sync_transaction{
        public:
            sync_transaction(sync_client_t client)
            {
                set_sync_client(client);
            }
            virtual ~sync_transaction(){

            }

            bool DISCARD(){
                SYNC_CALL(false, "DISCARD");

                return SYNC_HEADER_TYPE == e_type_ok;
            }

            bool EXEC(){
                SYNC_CALL(false, "EXEC");

                return true;
            }

            bool MULTI(){
                SYNC_CALL(false, "MULTI");

                return SYNC_HEADER_TYPE == e_type_ok;
            }


            bool UNWATCH(){
                SYNC_CALL(false, "UNWATCH");

                return SYNC_HEADER_TYPE == e_type_ok;
            }

            template<class ...ARGS>
            bool WATCH(ARGS ... args){
                SYNC_CALL(false, "WATCH", args...);

                return SYNC_HEADER_TYPE == e_type_ok;
            }

        private:
            SHINE_GEN_MEMBER_GETSET(sync_client_t, sync_client, = nullptr);
        };
    }
}
