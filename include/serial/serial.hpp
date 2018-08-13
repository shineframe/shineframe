 /**
 *****************************************************************************
 *
 *@note shineframe开发框架 https://github.com/shineframe/shineframe
 *
 *@file SERIAL.hpp
 *
 *@brief 序列化库 -- shine SERIAL
 *
 *使用SHINE_SERIAL_MODEL宏，只需要一行代码即可完成C++对象序列化/反序列化操作
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
#include <string>
#include <cassert>
#include <iostream>
#include <vector>
#include <list>
#include <deque>
#include <forward_list>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <vector>
#include <memory>
#include <functional>
#include <type_traits>
#include <ctype.h>
#include <stdlib.h>
#include <initializer_list>
#include "../common/define.hpp"
#include "../util/string.hpp"

#if (defined SHINE_OS_WINDOWS)
#pragma warning(disable:4146)
#endif

#define SERIAL_GEN_ENCODE_FUNC(N, ...) \
    MARCO_EXPAND_WARP(MAKE_ARG_LIST(N, SERIAL_ENCODE_FIELD, __VA_ARGS__))

#define SERIAL_GEN_DECODE_FUNC(N, ...) \
    MARCO_EXPAND_WARP(MAKE_ARG_LIST(N, SERIAL_DECODE_FIELD, __VA_ARGS__))

#define SERIAL_ENCODE(...) SERIAL_GEN_ENCODE_FUNC(GET_ARG_COUNT(__VA_ARGS__), __VA_ARGS__)
#define SERIAL_DECODE(...) SERIAL_GEN_DECODE_FUNC(GET_ARG_COUNT(__VA_ARGS__), __VA_ARGS__)


#define SERIAL_ENCODE_FIELD(field)\
{\
    no++;\
    shine::string tmp = serial_encode_field(this->field);\
    if (!tmp.empty())\
    {\
        ret += serial_encode_size(no);\
        ret += tmp;\
        size++;\
    }\
}

#define SERIAL_DECODE_FIELD(field) \
{\
    no++;\
    if (no == data_no)\
    {\
        if (!serial_decode_field(this->field, data, len, cost_len)) return false;\
        if (--count == 0){\
        cost_len = flag + size;\
        return true;\
        }\
        if (!serial_decode_size(data_no, data, len, cost_len)) return false; \
    }\
}



#define SHINE_SERIAL_MODEL(TYPE, ...) inline shine::string serial_encode(){\
    shine::string ret; \
    shine::size_t size = 0;\
    shine::size_t no = 0;\
    SERIAL_ENCODE(__VA_ARGS__); \
    if (size > 0){\
        ret.insert(0, serial_encode_size(size));\
        ret.insert(0, serial_encode_size(ret.size()));\
        ret.insert(0, 1, (shine::int8)shine::serial::e_struct); \
    }\
    return std::move(ret); \
    }\
    \
    inline bool serial_decode(const shine::string &data){\
        shine::size_t cost_len = 0;\
        return serial_decode(data.data(), data.size(), cost_len);\
    } \
    \
    inline bool serial_decode(const shine::int8 *data, const shine::size_t len, shine::size_t &cost_len){\
        if (data[cost_len] != shine::serial::e_struct) return false; \
        shine::size_t size = 0; \
        if (!serial_decode_size(size, data, len, ++cost_len)) return false; \
        shine::size_t flag = cost_len;\
        if (len - cost_len < size) return false; \
        shine::size_t count = 0; \
        if (!serial_decode_size(count, data, len, cost_len)) return false; \
        if (count == 0) return true; \
        shine::size_t no = 0; \
        shine::size_t data_no = 0; \
        if (!serial_decode_size(data_no, data, len, cost_len)) return false; \
        SERIAL_DECODE(__VA_ARGS__); \
        cost_len = flag + size;\
        return true; \
    } \
    }; \
    inline shine::string serial_encode_field(TYPE &val, bool check = true){ return val.serial_encode(); }\
    inline bool serial_decode_field(TYPE &val, const shine::int8 *data, const shine::size_t len, shine::size_t &cost_len){\
        return val.serial_decode(data, len, cost_len);


namespace shine
{
    namespace serial
    {
        enum{
            e_bool = 0,
            e_byte = 1,
            e_integer = 2,
            e_float = 3,
            e_double = 4,
            e_bytes = 5,
            e_array = 6,
            e_set = 7,
            e_map = 8,
            e_struct = 9,
        };

        template<typename T>
        inline void encode_size(shine::string &buf, T size){
            do {
                shine::int8 ch = size & ((1 << 7) - 1);
                if (size >>= 7)
                    ch |= 0x80;

                buf += ch;
            } while (size);
        }

        template<typename T>
        inline shine::size_t decode_size(T &val, const shine::int8 *data, const shine::size_t len){
            if (len < 1)
                return 0;

            val = 0;
            const shine::int8 *p = data;
            shine::size_t i = 0;

            for (;;)
            {
                if ((p[i] & 0x80) == 0x80)
                {
                    if (i < len - 1)
                    {
                        val += (p[i] & ((1 << 7) - 1)) * (1 << 7 * i);
                        i++;
                    }
                    else
                        return 0;
                }
                else
                {
                    val += (p[i] & ((1 << 7) - 1)) * (1 << 7 * i);
                    return i + 1;
                }
            }
        }

    }

    struct package{
        uint32 length = 0;
        uint32 type = 0;

        inline string encode(){
            string ret;

            serial::encode_size(ret, length);
            serial::encode_size(ret, type);

            return std::move(ret);
        }

        inline size_t decode(const int8 *data, const size_t len){
            size_t cost_len = serial::decode_size(length, data, len);

            if (cost_len == 0)
                return 0;

            size_t cost_len2 = serial::decode_size(type, data + cost_len, len - cost_len);

            if (cost_len2 == 0)
                return 0;

            return cost_len + cost_len2;
        }
    };

}

inline void serial_encode_type(shine::int8 type, shine::string &buf){
    buf += type;
}

#define SERIAL_ENCODE_TYPE(type) serial_encode_type(type, ret);

#define SERIAL_DECODE_BASE_CHECK(type) if (len < cost_len + 2 || data[cost_len] != type) return false;

inline shine::string serial_encode_size(shine::size_t len){
    shine::string ret;
    do {
        shine::int8 ch = len & ((1 << 7) - 1);
        if (len >>= 7)
            ch |= 0x80;

        ret += ch;
    } while (len);

    return std::move(ret);
}

inline bool serial_decode_size(shine::size_t &val, const shine::int8 *data, const shine::size_t len, shine::size_t &cost_len){
    if (len < cost_len + 1) 
        return false;

    val = 0;
    const shine::int8 *p = data + cost_len;
    shine::size_t i = 0;

    for (;;)
    {
        if ((p[i] & 0x80) == 0x80)
        {
            if (i < len - cost_len - 1)
            {
                val += (p[i] & ((1 << 7) - 1)) * (1 << 7 * i);
                i++;
            }
            else
                return false;
        }
        else
        {
            val += (p[i] & ((1 << 7) - 1)) * (1 << 7 * i);
            cost_len += i + 1;
            return true;
        }
    }
}

template<class T>
shine::string serial_encode_integer(T val, bool check = true){
    shine::string ret;
    if (check && val == 0)
        return std::move(ret);

    SERIAL_ENCODE_TYPE(shine::serial::e_integer);

    bool flag = val < 0;

    shine::uint64 value = flag ? -val : val;
    do {
        shine::int8 ch = value & ((1 << 6) - 1);
        if (value >>= 6)
            ch |= 0x80;

        ret += ch;
    } while (value);

    if (flag)
        ret[1] |= 0x40;

    return std::move(ret);
}


template<class T>
bool serial_decode_integer(T &val, const shine::int8 *data, const shine::size_t len, shine::size_t &cost_len){
    SERIAL_DECODE_BASE_CHECK(shine::serial::e_integer);

    val = 0;

    const shine::int8 *p = data + cost_len + 1;
    shine::Bool flag = (p[0] & 0x40) == 0x40;
    shine::size_t i = 0;

    for (;;)
    {
        if ((p[i] & 0x80) == 0x80)
        {
            if (i < len - cost_len - 1)
            {
                val += (p[i] & ((1 << 6) - 1)) * (1 << 6 * i);
                i++;
            }
            else
                return false;
        }
        else
        {
            val += (p[i] & ((1 << 6) - 1)) * (1 << 6 * i);
            break;
        }
    }

    if (flag)
        val = -val;

    cost_len += i + 2;
    return true;
}

inline shine::string serial_encode_field(shine::Bool val, bool check = true){
    shine::string ret;
    SERIAL_ENCODE_TYPE(shine::serial::e_bool);
    ret += val ? '\1' : '\0';
    return std::move(ret);
}

inline bool serial_decode_field(shine::Bool &val, const shine::int8 *data, const shine::size_t len, shine::size_t &cost_len){
    SERIAL_DECODE_BASE_CHECK(shine::serial::e_bool);

    val = data[cost_len + 1] == '\1';
    cost_len += 2;
    return true;
}

inline shine::string serial_encode_field(shine::int8 val, bool check = true){
    shine::string ret;
    SERIAL_ENCODE_TYPE(shine::serial::e_byte); 
    ret += val;
    return std::move(ret);
}

inline bool serial_decode_field(shine::int8 &val, const shine::int8 *data, const shine::size_t len, shine::size_t &cost_len){
    SERIAL_DECODE_BASE_CHECK(shine::serial::e_byte);

    val = data[cost_len + 1];
    cost_len += 2;
    return true;
}

inline shine::string serial_encode_field(shine::uint8 val, bool check = true){
    shine::string ret;
    SERIAL_ENCODE_TYPE(shine::serial::e_byte);
    ret += val;
    return std::move(ret);
}

inline bool serial_decode_field(shine::uint8 &val, const shine::int8 *data, const shine::size_t len, shine::size_t &cost_len){
    SERIAL_DECODE_BASE_CHECK(shine::serial::e_byte);

    val = data[cost_len + 1];
    cost_len += 2;
    return true;
}

#define SERIAL_ENCODE_BYTES_FIELD(TYPE) \
    inline shine::string serial_encode_field(const TYPE &val, bool check = true){\
        shine::string ret; \
        if (check  && val.empty()) return std::move(ret);\
        SERIAL_ENCODE_TYPE(shine::serial::e_bytes);\
        ret += serial_encode_size(val.size()); \
        ret.append(val);\
        return std::move(ret); \
    }

SERIAL_ENCODE_BYTES_FIELD(shine::string);
SERIAL_ENCODE_BYTES_FIELD(std::string);

#define SERIAL_DECODE_BYTES_FIELD(TYPE) \
    inline bool serial_decode_field(TYPE &val, const shine::int8 *data, const shine::size_t len, shine::size_t &cost_len){\
        SERIAL_DECODE_BASE_CHECK(shine::serial::e_bytes);\
        shine::size_t size = 0; \
        if (!serial_decode_size(size, data, len, ++cost_len)) return false;\
        if (size + cost_len > len) return false;\
        val.assign(data + cost_len, size); \
        cost_len += size; \
        return true; \
    }

SERIAL_DECODE_BYTES_FIELD(shine::string);
SERIAL_DECODE_BYTES_FIELD(std::string);

#define SERIAL_ENCODE_INTEGER_FIELD(TYPE) \
    inline shine::string serial_encode_field(const TYPE &val, bool check = true){\
        return serial_encode_integer(val, check); \
    }

#define SERIAL_DECODE_INTEGER_FIELD(TYPE) \
    inline bool serial_decode_field(TYPE &val, const shine::int8 *data, const shine::size_t len, shine::size_t &cost_len){\
        return serial_decode_integer(val, data, len, cost_len); \
    }

SERIAL_ENCODE_INTEGER_FIELD(shine::int16);
SERIAL_DECODE_INTEGER_FIELD(shine::int16);

SERIAL_ENCODE_INTEGER_FIELD(shine::uint16);
SERIAL_DECODE_INTEGER_FIELD(shine::uint16);

SERIAL_ENCODE_INTEGER_FIELD(shine::int32);
SERIAL_DECODE_INTEGER_FIELD(shine::int32);

SERIAL_ENCODE_INTEGER_FIELD(shine::Long);
SERIAL_DECODE_INTEGER_FIELD(shine::Long);

SERIAL_ENCODE_INTEGER_FIELD(shine::uLong);
SERIAL_DECODE_INTEGER_FIELD(shine::uLong);

SERIAL_ENCODE_INTEGER_FIELD(shine::uint32);
SERIAL_DECODE_INTEGER_FIELD(shine::uint32);

SERIAL_ENCODE_INTEGER_FIELD(shine::int64);
SERIAL_DECODE_INTEGER_FIELD(shine::int64);

SERIAL_ENCODE_INTEGER_FIELD(shine::uint64);
SERIAL_DECODE_INTEGER_FIELD(shine::uint64);

#define SERIAL_ENCODE_FLOAT_FIELD(TYPE, EN)\
inline shine::string serial_encode_field(const TYPE &val, bool check = true){\
    shine::string ret;\
    if (check && val == (TYPE)0.0) return std::move(ret);\
    SERIAL_ENCODE_TYPE(EN);\
    ret.append((const shine::int8*)&val, sizeof(val));\
    return std::move(ret); \
}

#define SERIAL_DECODE_FLOAT_FIELD(TYPE, EN)\
inline bool serial_decode_field(TYPE &val, const shine::int8 *data, const shine::size_t len, shine::size_t &cost_len){\
    if (len < cost_len + 1 + sizeof(val) || data[cost_len] != EN)\
        return false;\
        \
    val = *((TYPE*)(data + cost_len + 1));\
    cost_len += 1 + sizeof(val);\
    return true;\
}

SERIAL_ENCODE_FLOAT_FIELD(shine::Float, shine::serial::e_float);
SERIAL_DECODE_FLOAT_FIELD(shine::Float, shine::serial::e_float);

SERIAL_ENCODE_FLOAT_FIELD(shine::Double, shine::serial::e_double);
SERIAL_DECODE_FLOAT_FIELD(shine::Double, shine::serial::e_double);

SERIAL_ENCODE_FLOAT_FIELD(shine::LDouble, shine::serial::e_double);
SERIAL_DECODE_FLOAT_FIELD(shine::LDouble, shine::serial::e_double);

#define SERIAL_ENCODE_MAP_FIELD(TYPE) \
    template<typename T1, typename T2>\
    inline shine::string serial_encode_field(TYPE<T1, T2> &val, bool check = true){\
        shine::string ret;\
        if (check && val.empty()) return std::move(ret);\
        SERIAL_ENCODE_TYPE(shine::serial::e_map);\
        ret += serial_encode_size(val.size());\
        \
        for (auto &iter : val)\
        {\
            ret += serial_encode_field(iter.first, false); \
            ret += serial_encode_field(iter.second, false); \
        }\
        return std::move(ret);\
    }

#define SERIAL_DECODE_MAP_FIELD(TYPE) \
    template<typename T1, typename T2>\
    inline bool serial_decode_field(TYPE<T1, T2> &val, const shine::int8 *data, const shine::size_t len, shine::size_t &cost_len){\
        SERIAL_DECODE_BASE_CHECK(shine::serial::e_map); \
        shine::size_t size = 0; \
        if (!serial_decode_size(size, data, len, ++cost_len)) return false; \
        for (shine::size_t i = 0; i < size; i++)\
        {\
            T1 k;\
            if (!serial_decode_field(k, data, len, cost_len)) return false;\
            T2 v;\
            if (!serial_decode_field(v, data, len, cost_len)) return false;\
            val.emplace(std::move(k), std::move(v));\
        }\
        return true; \
    }


SERIAL_ENCODE_MAP_FIELD(std::map);
SERIAL_ENCODE_MAP_FIELD(std::unordered_map);

SERIAL_DECODE_MAP_FIELD(std::map);
SERIAL_DECODE_MAP_FIELD(std::unordered_map);


#define SERIAL_ENCODE_ARRAY_FIELD(TYPE) \
    template<typename T>\
    inline shine::string serial_encode_field(TYPE<T> &val, bool check = true){\
        shine::string ret;\
        if (check && val.empty()) return std::move(ret);\
        SERIAL_ENCODE_TYPE(shine::serial::e_array);\
        ret += serial_encode_size(val.size());\
        for (auto &iter : val) ret += serial_encode_field(iter, false); \
        return std::move(ret);\
    }

#define SERIAL_DECODE_ARRAY_FIELD(TYPE) \
    template<typename T>\
    inline bool serial_decode_field(TYPE<T> &val, const shine::int8 *data, const shine::size_t len, shine::size_t &cost_len){\
        SERIAL_DECODE_BASE_CHECK(shine::serial::e_array); \
        shine::size_t size = 0; \
        if (!serial_decode_size(size, data, len, ++cost_len)) return false; \
        for (shine::size_t i = 0; i < size; i++)\
        {\
            T v;\
            if (!serial_decode_field(v, data, len, cost_len)) return false;\
            val.emplace_back(std::move(v)); \
        }\
        return true; \
    }

SERIAL_ENCODE_ARRAY_FIELD(std::vector);
SERIAL_DECODE_ARRAY_FIELD(std::vector);

SERIAL_ENCODE_ARRAY_FIELD(std::deque);
SERIAL_DECODE_ARRAY_FIELD(std::deque);

SERIAL_ENCODE_ARRAY_FIELD(std::list);
SERIAL_DECODE_ARRAY_FIELD(std::list);


template<typename T>
inline shine::string serial_encode_field(std::forward_list<T> &val, bool check = true){
        shine::string ret; 
        if (check && val.empty()) return std::move(ret);
        SERIAL_ENCODE_TYPE(shine::serial::e_array); 
        shine::size_t size = 0;
        for (auto &iter : val) {
            ret.insert(1, serial_encode_field(iter, false));
            size++;
        }

        ret.insert(1, serial_encode_size(size));
        return std::move(ret); 
}

template<typename T>
inline bool serial_decode_field(std::forward_list<T> &val, const shine::int8 *data, const shine::size_t len, shine::size_t &cost_len){
        SERIAL_DECODE_BASE_CHECK(shine::serial::e_array); 
        shine::size_t size = 0;
        if (!serial_decode_size(size, data, len, ++cost_len)) return false;
        for (shine::size_t i = 0; i < size; i++)
        {
            T v;
            if (!serial_decode_field(v, data, len, cost_len)) return false;
            val.emplace_front(std::move(v)); 
        }
        return true; 
    }

#define SERIAL_ENCODE_SET_FIELD(TYPE) \
    template<typename T>\
    inline shine::string serial_encode_field(TYPE<T> &val, bool check = true){\
        shine::string ret;\
        if (check && val.empty()) return std::move(ret);\
        SERIAL_ENCODE_TYPE(shine::serial::e_set);\
        ret += serial_encode_size(val.size());\
        for (auto &iter : val) ret += serial_encode_field(iter, false); \
        return std::move(ret);\
    }

#define SERIAL_DECODE_SET_FIELD(TYPE) \
    template<typename T>\
    inline bool serial_decode_field(TYPE<T> &val, const shine::int8 *data, const shine::size_t len, shine::size_t &cost_len){\
        SERIAL_DECODE_BASE_CHECK(shine::serial::e_set); \
        shine::size_t size = 0; \
        if (!serial_decode_size(size, data, len, ++cost_len)) return false; \
        for (shine::size_t i = 0; i < size; i++)\
        {\
            T v;\
            if (!serial_decode_field(v, data, len, cost_len)) return false;\
            val.emplace(std::move(v)); \
        }\
        return true; \
    }

SERIAL_ENCODE_SET_FIELD(std::set);
SERIAL_DECODE_SET_FIELD(std::set);

SERIAL_ENCODE_SET_FIELD(std::unordered_set);
SERIAL_DECODE_SET_FIELD(std::unordered_set);

