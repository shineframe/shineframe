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

        class sync_connection{
        public:
            sync_connection(sync_client_t client)
            {
                set_sync_client(client);
            }
            virtual ~sync_connection(){

            }

            bool AUTH(const string &pass){
                SYNC_CALL(false, "AUTH", pass);

                return SYNC_HEADER_TYPE == e_type_ok;
            }

            bool ECHO(const string &message, string &reply_message){
                SYNC_CALL(false, "ECHO", message);

                result_parse::bulk_string_reply(SYNC_DATA_VALUE, reply_message);
                return message == reply_message;
            }

            bool PING(){
                SYNC_CALL(false, "PING");

                return SYNC_HEADER_VALUE == "PONG";
            }

            bool QUIT(){
                SYNC_CALL(false, "QUIT");

                return SYNC_HEADER_TYPE == e_type_ok;
            }

            bool SELECT(uint8 database){
                SYNC_CALL(false, "SELECT", database);

                return SYNC_HEADER_TYPE == e_type_ok;
            }

        private:
            SHINE_GEN_MEMBER_GETSET(sync_client_t, sync_client, = nullptr);
        };

    }
}
