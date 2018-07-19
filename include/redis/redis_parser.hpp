#pragma once

#include <iostream>
#include <chrono>
#include <ctime>
#include <string>
#include <mutex>
#include <memory>
#include <map>
#include <unordered_map>
#include <forward_list>
#include <vector>
#include <functional>
#include "../common/define.hpp"
#include "../util/pool.hpp"
#include "../util/string.hpp"

#if (defined SHINE_OS_WINDOWS)
#else
#endif

#define SYNC_CALL(ret, ...)  if (!get_sync_client()->call(request::encode({__VA_ARGS__}))) \
    return ret;\
    if (SYNC_HEADER_TYPE == e_type_error){\
        std::cout << __FUNCTION__ << "  " << SYNC_HEADER_VALUE << std::endl;\
        return ret;}


#define SYNC_DATA_VALUE get_sync_client()->get_response().get_data()
#define SYNC_HEADER_TYPE get_sync_client()->get_response().get_data().get_type()
#define SYNC_HEADER_VALUE get_sync_client()->get_response().get_data().get_value()
#define SYNC_CLIENTS_VALUE get_sync_client()->get_response().get_data().get_clients()

using namespace std;

namespace shine
{
    namespace redis
    {

        namespace decode_result{
            static const int32 success = 0;
            static const int32 not_enough_header = 1;
            static const int32 not_enough_total = 2;
            static const int32 parse_error = 3;
        }

        enum {
            e_decode_header = 0,
            e_decode_body = 1,
            e_decode_done = 2,
        };

        enum
        {
            e_type_ok,
            e_type_error,
            e_type_integer,
            e_type_bulk,
            e_type_multi_bulk,
            e_type_null,
        };

        enum
        {
            e_mode_sync,
            e_mode_async,
            e_mode_sync_pipe,
            e_mode_async_pipe,
        };

        struct reply_data;
        typedef reply_data* data_ptr_t;
        typedef std::vector<data_ptr_t> array_data_t;
        typedef std::shared_ptr<array_data_t> clients_data_t;

        static pool::simple<reply_data> reply_data_pool;

        struct reply_data
        {
            ~reply_data(){
                clear();
            }

            inline void clear()
            {
                if (get_clients())
                {
                    array_data_t& clients = *get_clients();
                    for (size_type i = 0; i < clients.size(); i++)
                        reply_data_pool.put(clients[i]);

                    get_clients().reset();
                }
            }

            SHINE_GEN_MEMBER_GETSET(data_ptr_t, parent, = nullptr);
            SHINE_GEN_MEMBER_GETSET(uint8, type, = e_type_bulk);
            SHINE_GEN_MEMBER_GETSET(string, value);
            SHINE_GEN_MEMBER_GETSET(clients_data_t, clients);

        };

        struct geo_info_t{
            string name;
            string longitude;
            string latitude;
            double distance = 0;
        };

        class header{
        private:

            SHINE_GEN_MEMBER_GETSET(reply_data, data);
        };

        class body{
        public:
        };

        class request
        {
        public:

            static string encode(std::vector<string> &&args)
            {
                string ret = "*";
                ret += (uint64)args.size();
                ret += "\r\n";

                for (auto &arg : args)
                {
                    ret += "$";
                    ret += (uint64)arg.size();
                    ret += "\r\n";
                    ret += arg;
                    ret += "\r\n";
                }

                return ret;
            }

        };

        class response : public header, public body
        {
        public:
            size_t decode_header(const int8 *data_, size_t len)
            {
                return 0;
            }

        private:
        };

        class result_parse{
        public:
            static void array_reply(reply_data &result, std::vector<string> &arr){
                if (result.get_clients())
                {
                    array_data_t &arr_data = *result.get_clients();
                    arr.resize(arr_data.size());
                    for (size_t i = 0; i < arr_data.size(); i++)
                    {
                        if (arr_data[i]->get_clients())
                        {
                            arr[i] = std::move(arr_data[i]->get_clients()->front()->get_value());
                        }
                        else
                        {
                            arr[i].clear();
                        }
                    }
                }
            }

            static void array_reply(reply_data &result, std::map<string, string> &map){
                map.clear();

                if (result.get_clients())
                {
                    array_data_t &arr_data = *result.get_clients();
                    for (size_t i = 0; i < arr_data.size(); i += 2)
                    {
                        map.emplace(std::move(arr_data[i]->get_clients()->front()->get_value()), std::move(arr_data[i + 1]->get_clients()->front()->get_value()));
                    }
                }
            }

            static void array_reply(reply_data &result, std::set<string> &set){
                set.clear();

                if (result.get_clients())
                {
                    array_data_t &arr_data = *result.get_clients();
                    for (size_t i = 0; i < arr_data.size(); i++)
                    {
                        set.emplace(std::move(arr_data[i]->get_clients()->front()->get_value()));
                    }
                }
            }

            static void array_reply(reply_data &result, std::vector<std::pair<string, double> > &arr){

                if (result.get_clients())
                {
                    array_data_t &arr_data = *result.get_clients();
                    arr.resize(arr_data.size() / 2);
                    for (size_t i = 0; i < arr_data.size(); i += 2)
                    {
                        arr[i/2] = std::make_pair(std::move(arr_data[i]->get_clients()->front()->get_value()), std::stod(arr_data[i + 1]->get_clients()->front()->get_value()));
                    }
                }
            }

            static void GEOPOS_reply(reply_data &result, std::vector<std::pair<string, string>> &arr){

                arr.clear();
                if (result.get_clients())
                {
                    
                    array_data_t &arr_1 = *result.get_clients();
                    arr.resize(arr_1.size());

                    for (size_t i = 0; i < arr_1.size(); i++)
                    {
                        if (!arr_1[i]->get_clients() || arr_1[i]->get_clients()->size() != 2)
                            continue;

                        array_data_t &arr_2 = *arr_1[i]->get_clients();
                        if (!arr_2[0]->get_clients() || arr_2[0]->get_clients()->size() != 1 ||
                            !arr_2[1]->get_clients() || arr_2[1]->get_clients()->size() != 1)
                            continue;

                        arr[i].first = std::move(arr_2[0]->get_clients()->front()->get_value());
                        arr[i].second = std::move(arr_2[1]->get_clients()->front()->get_value());
                    }
                }
            }

            static void GEORADIUS_reply(reply_data &result, std::vector<geo_info_t> &arr){

                arr.clear();
                if (result.get_clients())
                {

                    array_data_t &arr_1 = *result.get_clients();
                    arr.resize(arr_1.size());

                    for (size_t i = 0; i < arr_1.size(); i++)
                    {
                        if (!arr_1[i]->get_clients() || arr_1[i]->get_clients()->size() != 3)
                            continue;

                        array_data_t &arr_2 = *arr_1[i]->get_clients();
                        if (!arr_2[0]->get_clients() || arr_2[0]->get_clients()->size() != 1
                            || !arr_2[1]->get_clients() || arr_2[1]->get_clients()->size() != 1
                            || !arr_2[2]->get_clients() || arr_2[2]->get_clients()->size() != 2
                            )
                            continue;

                        arr[i].name = std::move(arr_2[0]->get_clients()->front()->get_value());
                        arr[i].distance = std::stod(arr_2[1]->get_clients()->front()->get_value());

                        array_data_t &arr_3 = *arr_2[2]->get_clients();
                        if (arr_3.size() != 2 || !arr_3[0]->get_clients() || arr_3[0]->get_clients()->size() != 1
                            || !arr_3[1]->get_clients() || arr_3[1]->get_clients()->size() != 1)
                            continue;

                        arr[i].longitude = arr_3[0]->get_clients()->front()->get_value();
                        arr[i].latitude = arr_3[1]->get_clients()->front()->get_value();
                    }
                }
            }

            static void bulk_string_reply(reply_data &result, string &v){
                if (result.get_clients() && result.get_clients()->size() > 0)
                    v = std::move(result.get_clients()->front()->get_value());
            }

            static void scan_reply(reply_data &result, uint64 &index, std::vector<string> &arr){

                if (result.get_clients() && result.get_clients()->size() == 2)
                {
                    array_data_t &arr_1 = *result.get_clients();
                    index = std::stoll(arr_1[0]->get_clients()->front()->get_value());

                    array_data_t &data_array = *(arr_1[1]->get_clients());
                    arr.resize(data_array.size());
                    for (size_t i = 0; i < data_array.size(); i++)
                    {
                        arr[i] = std::move(data_array[i]->get_clients()->front()->get_value());
                    }
                }
            }

            static void scan_reply(reply_data &result, uint64 &index, std::map<string, string> &map){

                map.clear();
                if (result.get_clients() && result.get_clients()->size() == 2)
                {
                    array_data_t &arr_1 = *result.get_clients();
                    index = std::stoll(arr_1[0]->get_clients()->front()->get_value());

                    array_data_t &data_array = *(arr_1[1]->get_clients());
                    for (size_t i = 0; i < data_array.size(); i += 2)
                    {
                        map.emplace(std::move(data_array[i]->get_clients()->front()->get_value()), std::move(data_array[i + 1]->get_clients()->front()->get_value()));
                    }
                }
            }

            static void scan_reply(reply_data &result, uint64 &index, std::vector<std::pair<string, double> > &arr){

                if (result.get_clients() && result.get_clients()->size() == 2)
                {
                    array_data_t &arr_1 = *result.get_clients();
                    index = std::stoll(arr_1[0]->get_clients()->front()->get_value());

                    array_data_t &data_array = *(arr_1[1]->get_clients());
                    arr.resize(data_array.size() / 2);
                    for (size_t i = 0; i < data_array.size(); i += 2)
                    {
                        arr[i / 2] = std::make_pair(std::move(data_array[i]->get_clients()->front()->get_value()), std::stod(data_array[i + 1]->get_clients()->front()->get_value()));
                    }
                }
            }

            static void scan_reply(reply_data &result, uint64 &index, std::set<string> &set){

                set.clear();
                if (result.get_clients() && result.get_clients()->size() == 2)
                {
                    array_data_t &arr_1 = *result.get_clients();
                    index = std::stoll(arr_1[0]->get_clients()->front()->get_value());

                    array_data_t &data_array = *(arr_1[1]->get_clients());
                    for (size_t i = 0; i < data_array.size(); i ++)
                    {
                        set.emplace(std::move(data_array[i]->get_clients()->front()->get_value()));
                    }
                }
            }


        };

    }
}
