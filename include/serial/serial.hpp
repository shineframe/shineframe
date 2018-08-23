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

#define SERIAL_GEN_COMPARE_FUNC(N, ...) \
    MARCO_EXPAND_WARP(MAKE_ARG_LIST(N, SERIAL_COMPARE_FIELD, __VA_ARGS__))

#define SERIAL_ENCODE(...) SERIAL_GEN_ENCODE_FUNC(GET_ARG_COUNT(__VA_ARGS__), __VA_ARGS__)
#define SERIAL_DECODE(...) SERIAL_GEN_DECODE_FUNC(GET_ARG_COUNT(__VA_ARGS__), __VA_ARGS__)
#define SERIAL_COMPARE(...) SERIAL_GEN_COMPARE_FUNC(GET_ARG_COUNT(__VA_ARGS__), __VA_ARGS__)


#define SERIAL_ENCODE_FIELD(field)\
{\
    serial_encode_field(this->field, ret);\
}

#define SERIAL_DECODE_FIELD(field) \
{\
        if (!serial_decode_field(this->field, data, len, cost_len)) return false;\
}

#define SERIAL_COMPARE_FIELD(field) \
{\
    if (!(a.field == b.field)) return false;\
}


#define SHINE_SERIAL_MODEL(TYPE, ...) \
TYPE(){}\
~TYPE(){}\
inline std::string serial_encode() const{\
    std::string ret;\
    serial_encode(ret);\
    return std::move(ret);\
}\
inline void serial_encode(std::string &ret) const{\
    shine::size_t old_size_flag = ret.size();\
    shine::size_t size = 0;\
    shine::size_t no = 0;\
    SERIAL_ENCODE(__VA_ARGS__); \
    serial_encode_size(ret.size() - old_size_flag, ret, false, old_size_flag); \
    }\
    \
    inline bool serial_decode(const std::string &data){\
        shine::size_t cost_len = 0;\
        return serial_decode(data.data(), data.size(), cost_len);\
    } \
    inline bool serial_decode(const shine::int8 *data, const shine::size_t len){\
        shine::size_t cost_len = 0;\
        return serial_decode(data, len, cost_len);\
    } \
    \
    inline bool serial_decode(const shine::int8 *data, const shine::size_t len, shine::size_t &cost_len){\
        if (len - cost_len == 0) return true;\
        shine::size_t size = 0; \
        if (!serial_decode_size(size, data, len, cost_len)) return false; \
        if (size == 0) return true;\
        shine::size_t flag = cost_len;\
        if (len - cost_len < size) return false; \
        SERIAL_DECODE(__VA_ARGS__); \
        cost_len = flag + size;\
        return true; \
    } \
    }; \
    inline bool operator==(const TYPE &a, const TYPE &b){\
        SERIAL_COMPARE(__VA_ARGS__); \
        return true;\
    }\
    inline bool operator!=(const TYPE &a, const TYPE &b){\
        return !(a == b);\
    }\
    inline void serial_encode_field(const TYPE &val, std::string &ret){ return val.serial_encode(ret); }\
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
        inline void encode_size(std::string &buf, T size){
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

    struct package_t{
        size_t length = 0;
        size_t type = 0;

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

#define SERIAL_DECODE_BASE_CHECK(type) if (len < cost_len + 1) return false;

inline void serial_encode_size(shine::size_t len, std::string &ret, bool back = true, shine::size_t pos = 0){
    do {
        shine::int8 ch = len & ((1 << 7) - 1);
        if (len >>= 7)
            ch |= 0x80;

        if (back) 
        {
            ret += ch;
        }
        else
        {
            ret.insert(pos++, 1, ch);
        }
    } while (len);
}

inline std::string serial_encode_size(shine::size_t len){
    std::string ret;
    serial_encode_size(len, ret);
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
void serial_encode_integer(T val, std::string &ret){
    bool flag = val < 0;
    shine::size_t size = ret.size();
    shine::uint64 value = flag ? -val : val;
    do {
        shine::int8 ch = value & ((1 << 6) - 1);
        if (value >>= 6)
            ch |= 0x80;

        ret += ch;
    } while (value);

    if (flag)
        ret[size] |= 0x40;
}


template<class T>
bool serial_decode_integer(T &val, const shine::int8 *data, const shine::size_t len, shine::size_t &cost_len){
    val = 0;
    const shine::int8 *p = data + cost_len;
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

    cost_len += i + 1;
    return true;
}

inline void serial_encode_field(shine::Bool val, std::string &ret){
    ret += (shine::int8)(val ? '\1' : '\0');
}

inline bool serial_decode_field(shine::Bool &val, const shine::int8 *data, const shine::size_t len, shine::size_t &cost_len){
    SERIAL_DECODE_BASE_CHECK(shine::serial::e_bool);

    val = data[cost_len++] == '\1';
    return true;
}

inline void serial_encode_field(shine::int8 val, std::string &ret){
    ret += val;
}

inline bool serial_decode_field(shine::int8 &val, const shine::int8 *data, const shine::size_t len, shine::size_t &cost_len){
    SERIAL_DECODE_BASE_CHECK(shine::serial::e_byte);

    val = data[cost_len++];
    return true;
}

inline void serial_encode_field(shine::uint8 val, std::string &ret){
    ret += val;
}

inline bool serial_decode_field(shine::uint8 &val, const shine::int8 *data, const shine::size_t len, shine::size_t &cost_len){
    SERIAL_DECODE_BASE_CHECK(shine::serial::e_byte);

    val = data[cost_len++];
    return true;
}

#define SERIAL_ENCODE_BYTES_FIELD(TYPE) \
    inline void serial_encode_field(const TYPE &val, std::string &ret){\
        serial_encode_size(val.size(), ret); \
        ret.append(val);\
    }

SERIAL_ENCODE_BYTES_FIELD(std::string);
SERIAL_ENCODE_BYTES_FIELD(shine::string);

#define SERIAL_DECODE_BYTES_FIELD(TYPE) \
    inline bool serial_decode_field(TYPE &val, const shine::int8 *data, const shine::size_t len, shine::size_t &cost_len){\
        shine::size_t size = 0; \
        if (!serial_decode_size(size, data, len, cost_len)) return false;\
        if (size + cost_len > len) return false;\
        val.assign(data + cost_len, size); \
        cost_len += size; \
        return true; \
    }

SERIAL_DECODE_BYTES_FIELD(std::string);
SERIAL_DECODE_BYTES_FIELD(shine::string);

#define SERIAL_ENCODE_INTEGER_FIELD(TYPE) \
    inline void serial_encode_field(const TYPE &val, std::string &ret){\
        return serial_encode_integer(val, ret); \
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

#define SERIAL_ENCODE_FLOAT_FIELD(TYPE)\
inline void serial_encode_field(const TYPE &val, std::string &ret){\
    ret.append((const shine::int8*)&val, sizeof(val));\
}

#define SERIAL_DECODE_FLOAT_FIELD(TYPE)\
inline bool serial_decode_field(TYPE &val, const shine::int8 *data, const shine::size_t len, shine::size_t &cost_len){\
    if (len < cost_len + sizeof(val))\
        return false;\
        \
    val = *((TYPE*)(data + cost_len));\
    cost_len += sizeof(val);\
    return true;\
}

SERIAL_ENCODE_FLOAT_FIELD(shine::Float);
SERIAL_DECODE_FLOAT_FIELD(shine::Float);

SERIAL_ENCODE_FLOAT_FIELD(shine::Double);
SERIAL_DECODE_FLOAT_FIELD(shine::Double);

SERIAL_ENCODE_FLOAT_FIELD(shine::LDouble);
SERIAL_DECODE_FLOAT_FIELD(shine::LDouble);

#define SERIAL_ENCODE_MAP_FIELD(TYPE) \
    template<typename T1, typename T2>\
    inline void serial_encode_field(const TYPE<T1, T2> &val, std::string &ret){\
        serial_encode_size(val.size(), ret); \
        for (auto &iter : val)\
        {\
            serial_encode_field(iter.first, ret); \
            serial_encode_field(iter.second, ret); \
        }\
    }

#define SERIAL_DECODE_MAP_FIELD(TYPE) \
    template<typename T1, typename T2>\
    inline bool serial_decode_field(TYPE<T1, T2> &val, const shine::int8 *data, const shine::size_t len, shine::size_t &cost_len){\
        shine::size_t size = 0; \
        if (!serial_decode_size(size, data, len, cost_len)) return false; \
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
    inline void serial_encode_field(const TYPE<T> &val, std::string &ret){\
        serial_encode_size(val.size(), ret); \
        for (auto &iter : val) serial_encode_field(iter, ret); \
    }

#define SERIAL_DECODE_ARRAY_FIELD(TYPE) \
    template<typename T>\
    inline bool serial_decode_field(TYPE<T> &val, const shine::int8 *data, const shine::size_t len, shine::size_t &cost_len){\
        shine::size_t size = 0; \
        if (!serial_decode_size(size, data, len, cost_len)) return false; \
        val.resize(size);\
        for (shine::size_t i = 0; i < size; i++)\
        {\
            if (!serial_decode_field(val[i], data, len, cost_len)) return false; \
        }\
        return true; \
    }

SERIAL_ENCODE_ARRAY_FIELD(std::vector);
SERIAL_DECODE_ARRAY_FIELD(std::vector);

SERIAL_ENCODE_ARRAY_FIELD(std::deque);
SERIAL_DECODE_ARRAY_FIELD(std::deque);

template<typename T>
inline void serial_encode_field(const std::list<T> &val, std::string &ret){
    serial_encode_size(val.size(), ret); 
    for (auto &iter : val) {
        serial_encode_field(iter, ret);
    }
}

template<typename T>
inline bool serial_decode_field(std::list<T> &val, const shine::int8 *data, const shine::size_t len, shine::size_t &cost_len){
    shine::size_t size = 0;
    if (!serial_decode_size(size, data, len, cost_len)) return false;
    val.resize(size);
    auto iter = val.begin();
    for (shine::size_t i = 0; i < size; i++)
    {
        if (!serial_decode_field(*iter++, data, len, cost_len)) return false;
    }
    return true;
}


template<typename T>
inline void serial_encode_field(const std::forward_list<T> &val, std::string &ret){
        shine::size_t begin = ret.size();
        shine::size_t size = 0;
        for (auto &iter : val) {
            std::string tmp;
            serial_encode_field(iter, tmp);
            ret.insert(begin, std::move(tmp));
            size++;
        }

        serial_encode_size(size, ret, false, begin);
}

template<typename T>
inline bool serial_decode_field(std::forward_list<T> &val, const shine::int8 *data, const shine::size_t len, shine::size_t &cost_len){
        shine::size_t size = 0;
        if (!serial_decode_size(size, data, len, cost_len)) return false;
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
    inline void serial_encode_field(const TYPE<T> &val, std::string &ret){\
        ret += std::move(serial_encode_size(val.size())); \
        for (auto &iter : val) serial_encode_field(iter, ret); \
    }

#define SERIAL_DECODE_SET_FIELD(TYPE) \
    template<typename T>\
    inline bool serial_decode_field(TYPE<T> &val, const shine::int8 *data, const shine::size_t len, shine::size_t &cost_len){\
        shine::size_t size = 0; \
        if (!serial_decode_size(size, data, len, cost_len)) return false; \
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

