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

#if (defined SHINE_OS_WINDOWS)
#else
#endif



using namespace std;

namespace shine
{
    namespace redis
    {
        typedef sync_client* sync_client_t;
        typedef async_client* async_client_t;

        class sync_script{
        public:
            sync_script(sync_client_t client)
            {
                set_sync_client(client);
            }
            virtual ~sync_script(){

            }

        private:
            SHINE_GEN_MEMBER_GETSET(sync_client_t, sync_client, = nullptr);
        };

    }
}
