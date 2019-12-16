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
#include "../util/string.hpp"
#include "../net/proactor_engine.hpp"

#if (defined SHINE_OS_WINDOWS)
#else
#endif

using namespace std;

#define http_method_get "GET"
#define http_method_head "HEAD"
#define http_method_post "POST"
#define http_method_put "PUT"
#define http_method_delete_  "DELETE"
#define http_method_connect "CONNECT"
#define http_method_options "OPTIONS"
#define http_method_trace "TRACE"

namespace shine
{
    namespace http
    {
        struct entry_t
        {
            SHINE_GEN_MEMBER_GETSET(string, key);
            SHINE_GEN_MEMBER_GETSET(string, value);
        };

//         namespace method
//         {
//             static const int8* get = "GET";
//             static const int8* head = "HEAD";
//             static const int8* post = "POST";
//             static const int8* put = "PUT";
//             static const int8* delete_ = "DELETE";
//             static const int8* connect = "CONNECT";
//             static const int8* options = "OPTIONS";
//             static const int8* trace = "TRACE";
//         }

        namespace decode_result{
            static const int32 success = 0;
            static const int32 not_enough_header = 1;
            static const int32 not_enough_total = 2;
            static const int32 parse_error = 3;
        }

        static const int8* get_status_code_desc(uint16 status)
        {
            static const std::unordered_map<uint16, const int8*> desc_map = {
                { 101, "Switching Protocols" },
                { 200, "OK" },
                { 201, "Created" },
                { 202, "Accepted" },
                { 203, "Non-Authoritative Information (for DNS)" },
                { 204, "No Content" },
                { 205, "Reset Content" },
                { 206, "Partial Content" },
                { 300, "Multiple Choices" },
                { 301, "Moved Permanently" },
                { 302, "Moved Temporarily" },
                { 303, "See Other" },
                { 304, "Not Modified" },
                { 305, "Use Proxy" },
                { 307, "Redirect Keep Verb" },
                { 400, "Bad Request" },
                { 401, "Unauthorized" },
                { 402, "Payment Required" },
                { 403, "Forbidden" },
                { 404, "Not Found" },
                { 405, "Bad Request" },
                { 406, "Not Acceptable" },
                { 407, "Proxy Authentication Required" },
                { 408, "Request Timed-Out" },
                { 409, "Conflict" },
                { 410, "Gone" },
                { 411, "Length Required" },
                { 412, "Precondition Failed" },
                { 413, "Request Entity Too Large" },
                { 414, "Request, URI Too Large" },
                { 415, "Unsupported Media Type" },
                { 500, "Internal Server Error" },
                { 501, "Not Implemented" },
                { 502, "Bad Gateway" },
                { 503, "Server Unavailable" },
                { 504, "Gateway Timed-Out" },
                { 505, "HTTP Version not supported" }
            };

            auto iter = desc_map.find(status);
            return iter == std::end(desc_map) ? "unknown" : iter->second;
        }

        class header{
        public:
            typedef std::vector<entry_t> entrys_t;
            typedef std::map<string, string> parameters_t;
        public:

            void add_entry(const entry_t &entry){
                get_entrys().push_back(entry);
            }

            void add_entry(entry_t &&entry){
                get_entrys().emplace_back(entry);
            }

        private:
            SHINE_GEN_MEMBER_GETSET(string, version, = "HTTP/1.1")
            SHINE_GEN_MEMBER_GETSET(entrys_t, entrys);

            SHINE_GEN_MEMBER_GETSET(size_t, content_length, = 0);
        };

        class body{
            SHINE_GEN_MEMBER_GETSET(string, body);
        };

        enum {
            e_decode_header = 0,
            e_decode_body = 1,
            e_decode_done = 2,
        };

        class request : public header, public body
        {
        public:
            size_t decode_header(const int8 *data_, size_t len)
            {
                int8 *data = (int8*)data_;
                int8 *header_end = strstr(data, "\r\n\r\n");
                if (header_end == NULL)
                    return -1;

                //decode method url version
                int8 *tmp = strstr(data, " ");
                if (tmp == NULL)
                    return -1;

                get_method().assign(data, tmp - data);
                data = tmp + 1;

                tmp = strstr(data, " ");
                if (tmp == NULL)
                    return -1;

                get_url().assign(data, tmp - data);
                data = tmp + 1;

                auto pos = get_url().find("?");
                if (pos != string::npos)
                {
                    string parameters = get_url().substr(pos + 1, get_url().size());
                    size_type cost_len = 0;
                    while (cost_len < parameters.size())
                    {
                        auto pa_pos = parameters.find("&", cost_len);
                        if (pa_pos == string::npos)
                        {
                            pa_pos = parameters.size();
                        }

                        auto k_pos = parameters.find("=", cost_len);
                        if (k_pos != string::npos)
                        {
                            get_url_parameters().emplace(parameters.substr(cost_len, k_pos - cost_len), parameters.substr(k_pos + 1, pa_pos - k_pos - 1));
                        }

                        cost_len = pa_pos + 1;
                    }

                    get_url().resize(pos);
                }

                tmp = strstr(data, "\r\n");
                if (tmp == NULL)
                    return -1;

                get_version().assign(data, tmp - data);
                data = tmp + 2;

                //decode other entrys
                while(data < header_end + 2)
                {
                    entry_t entry;
                    tmp = strstr(data, ": ");
                    if (tmp == NULL)
                        return -1;

                    entry.get_key().assign(data, tmp - data);
                    data = tmp + 2;

                    tmp = strstr(data, "\r\n");
                    entry.get_value().assign(data, tmp - data);
                    data = tmp + 2;

                    if (entry.get_key() == "Content-Length")
                        get_content_length() = entry.get_value().to_uint64();
                    else
                        add_entry(std::move(entry));
                }

                return len;
            }

            bool encode(string &buf) const
            {
                if (get_method().empty() || get_url().empty() || get_version().empty())
                    return false;

                buf.clear();
				buf += get_method() + " " + get_url() + " " + get_version() + "\r\n";
				buf += "Host: " + get_host() + "\r\n";
				for (auto &entry : get_entrys())
                {
                    buf += entry.get_key() + ": " + entry.get_value() + "\r\n";
                }

                if (get_body().size() > 0)
                {
                    buf += "Content-Length: ";
                    buf += (uint64)get_body().size();
                    buf += "\r\n";
                }

                buf += "\r\n";
                buf += get_body();

                return true;
            }

            void clear()
            {
                get_content_length() = 0;
                get_entrys().clear();
                get_url_parameters().clear();
            }

        private:
            SHINE_GEN_MEMBER_GETSET(string, method);
            SHINE_GEN_MEMBER_GETSET(string, host);
            SHINE_GEN_MEMBER_GETSET(string, url);
            SHINE_GEN_MEMBER_GETSET(parameters_t, url_parameters);
//             SHINE_GEN_MEMBER_GETSET(bool, keep_alive);
        };

        class response : public header, public body
        {
        public:
			typedef shine::net::connection* connection_t;
            size_t decode_header(const int8 *data_, size_t len)
            {
                int8 *data = (int8*)data_;
                int8 *header_end = strstr(data, "\r\n\r\n");
                if (header_end == NULL)
                    return -1;

                //decode version status code status desc
                int8 *tmp = strstr(data, " ");
                if (tmp == NULL)
                    return -1;

                get_version().assign(data, tmp - data);
                data = tmp + 1;

                tmp = strstr(data, " ");
                if (tmp == NULL)
                    return -1;

                *tmp = '\0';
                get_status_code() = atoi(data);
                data = tmp + 1;

                tmp = strstr(data, "\r\n");
                if (tmp == NULL)
                    return -1;

                get_status_desc().assign(data, tmp - data);
                data = tmp + 2;

                //decode other entrys
                while (data < header_end + 2)
                {
                    entry_t entry;
                    tmp = strstr(data, ": ");
                    if (tmp == NULL)
                        return -1;

                    entry.get_key().assign(data, tmp - data);
                    data = tmp + 2;

                    tmp = strstr(data, "\r\n");
                    entry.get_value().assign(data, tmp - data);
                    data = tmp + 2;

					if (entry.get_key() == "Content-Length")
						get_content_length() = entry.get_value().to_uint64();
					else if (entry.get_key() == "Transfer-Encoding" && entry.get_value() == "chunked")
						set_is_chunk(true);
                    else
                        add_entry(std::move(entry));
                }

                return len;
            }

            bool encode(string &buf) const
            {
                if (get_version().empty())
                    return false;

                buf.clear();
                buf += get_version() + " ";
                buf += get_status_code();
                buf += " ";
                buf += get_status_code_desc(get_status_code());
                buf += "\r\n";

                for (auto &entry : get_entrys())
                {
                    buf += entry.get_key() + ": " + entry.get_value() + "\r\n";
                }

                if (get_body().size() > 0)
                {
                    buf += "Content-Length: ";
                    buf += (uint64)get_body().size();
                    buf += "\r\n";
                }

                buf += "\r\n";
                buf += get_body();

                return true;
            }

            void clear()
            {
                get_content_length() = 0;
                get_entrys().clear();
            }

        private:
            SHINE_GEN_MEMBER_GETSET(uint16, status_code, = 200);
			SHINE_GEN_MEMBER_GETSET(string, status_desc);
			SHINE_GEN_MEMBER_GETSET(bool, is_chunk, = false);
		};

    }
}
