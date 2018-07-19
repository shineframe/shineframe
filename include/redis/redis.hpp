 /**
 *****************************************************************************
 *
 *@note shineframe开发框架 https://github.com/shineframe/shineframe
 *
 *@file redis.hpp
 *
 *@brief redis客户端封装
 *
 *@todo 
 *
 *@author sunjian 39215174@qq.com
 *
 *@version 1.0
 *
 *@date 2018/6/14 
 *****************************************************************************
 */

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
#include "../common/define.hpp"
#include "redis_parser.hpp"
#include "redis_client.hpp"
#include "commands/connection.hpp"
#include "commands/key.hpp"
#include "commands/hash.hpp"
#include "commands/set.hpp"
#include "commands/strings.hpp"
#include "commands/sorted_set.hpp"
#include "commands/list.hpp"
#include "commands/hyper_log_log.hpp"
#include "commands/transaction.hpp"
#include "commands/geo.hpp"
#include "commands/pub_sub.hpp"
#include "commands/cluster.hpp"
#include "commands/script.hpp"

#if (defined SHINE_OS_WINDOWS)
#else
#endif

using namespace std;

namespace shine
{
    namespace redis
    {
        class sync : public sync_client, 
            public sync_key,
            public sync_connection,
            public sync_hash,
            public sync_set,
            public sync_string,
            public sync_sorted_set,
            public sync_list,
            public sync_hyper_log_log,
            public sync_transaction,
            public sync_geo,
            public sync_pub_sub,
            public sync_cluster,
            public sync_script
        {
        public:
            sync() : sync_key(this), sync_connection(this), sync_hash(this), sync_set(this), sync_string(this)
                , sync_sorted_set(this), sync_list(this), sync_hyper_log_log(this), sync_transaction(this)
                , sync_geo(this), sync_pub_sub(this), sync_cluster(this), sync_script(this)
            {

            }

            const string & get_err_info() const
            {
                return get_response().get_data().get_value();
            }
        };
    }
}
