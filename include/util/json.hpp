 /**
 *****************************************************************************
 *
 *@note shineframe开发框架 https://github.com/shineframe/shineframe
 *
 *@file json.hpp
 *
 *@brief json解析库 -- shine json model
 *
 *使用SHINE_JSON_MODEL宏，只需要一行代码即可完成C++对象与json字符串之间的互相转换
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
#include "type_traits.hpp"
#include "string.hpp"

#define MARCO_EXPAND(...) __VA_ARGS__
#define MAKE_ARG_LIST_1(op, arg, ...)   op(arg)
#define MAKE_ARG_LIST_2(op, arg, ...)   op(arg) MARCO_EXPAND(MAKE_ARG_LIST_1(op, __VA_ARGS__))
#define MAKE_ARG_LIST_3(op, arg, ...)   op(arg) MARCO_EXPAND(MAKE_ARG_LIST_2(op, __VA_ARGS__))
#define MAKE_ARG_LIST_4(op, arg, ...)   op(arg) MARCO_EXPAND(MAKE_ARG_LIST_3(op, __VA_ARGS__))
#define MAKE_ARG_LIST_5(op, arg, ...)   op(arg) MARCO_EXPAND(MAKE_ARG_LIST_4(op, __VA_ARGS__))
#define MAKE_ARG_LIST_6(op, arg, ...)   op(arg) MARCO_EXPAND(MAKE_ARG_LIST_5(op, __VA_ARGS__))
#define MAKE_ARG_LIST_7(op, arg, ...)   op(arg) MARCO_EXPAND(MAKE_ARG_LIST_6(op, __VA_ARGS__))
#define MAKE_ARG_LIST_8(op, arg, ...)   op(arg) MARCO_EXPAND(MAKE_ARG_LIST_7(op, __VA_ARGS__))
#define MAKE_ARG_LIST_9(op, arg, ...)   op(arg) MARCO_EXPAND(MAKE_ARG_LIST_8(op, __VA_ARGS__))
#define MAKE_ARG_LIST_10(op, arg, ...)  op(arg) MARCO_EXPAND(MAKE_ARG_LIST_9(op, __VA_ARGS__))
#define MAKE_ARG_LIST_11(op, arg, ...)  op(arg) MARCO_EXPAND(MAKE_ARG_LIST_10(op, __VA_ARGS__))
#define MAKE_ARG_LIST_12(op, arg, ...)  op(arg) MARCO_EXPAND(MAKE_ARG_LIST_11(op, __VA_ARGS__))
#define MAKE_ARG_LIST_13(op, arg, ...)  op(arg) MARCO_EXPAND(MAKE_ARG_LIST_12(op, __VA_ARGS__))
#define MAKE_ARG_LIST_14(op, arg, ...)  op(arg) MARCO_EXPAND(MAKE_ARG_LIST_13(op, __VA_ARGS__))
#define MAKE_ARG_LIST_15(op, arg, ...)  op(arg) MARCO_EXPAND(MAKE_ARG_LIST_14(op, __VA_ARGS__))
#define MAKE_ARG_LIST_16(op, arg, ...)  op(arg) MARCO_EXPAND(MAKE_ARG_LIST_15(op, __VA_ARGS__))
#define MAKE_ARG_LIST_17(op, arg, ...)  op(arg) MARCO_EXPAND(MAKE_ARG_LIST_16(op, __VA_ARGS__))
#define MAKE_ARG_LIST_18(op, arg, ...)  op(arg) MARCO_EXPAND(MAKE_ARG_LIST_17(op, __VA_ARGS__))
#define MAKE_ARG_LIST_19(op, arg, ...)  op(arg) MARCO_EXPAND(MAKE_ARG_LIST_18(op, __VA_ARGS__))
#define MAKE_ARG_LIST_20(op, arg, ...)  op(arg) MARCO_EXPAND(MAKE_ARG_LIST_19(op, __VA_ARGS__))
#define MAKE_ARG_LIST_21(op, arg, ...)  op(arg) MARCO_EXPAND(MAKE_ARG_LIST_20(op, __VA_ARGS__))
#define MAKE_ARG_LIST_22(op, arg, ...)  op(arg) MARCO_EXPAND(MAKE_ARG_LIST_21(op, __VA_ARGS__))
#define MAKE_ARG_LIST_23(op, arg, ...)  op(arg) MARCO_EXPAND(MAKE_ARG_LIST_22(op, __VA_ARGS__))
#define MAKE_ARG_LIST_24(op, arg, ...)  op(arg) MARCO_EXPAND(MAKE_ARG_LIST_23(op, __VA_ARGS__))
#define MAKE_ARG_LIST_25(op, arg, ...)  op(arg) MARCO_EXPAND(MAKE_ARG_LIST_24(op, __VA_ARGS__))
#define MAKE_ARG_LIST_26(op, arg, ...)  op(arg) MARCO_EXPAND(MAKE_ARG_LIST_25(op, __VA_ARGS__))
#define MAKE_ARG_LIST_27(op, arg, ...)  op(arg) MARCO_EXPAND(MAKE_ARG_LIST_26(op, __VA_ARGS__))
#define MAKE_ARG_LIST_28(op, arg, ...)  op(arg) MARCO_EXPAND(MAKE_ARG_LIST_27(op, __VA_ARGS__))
#define MAKE_ARG_LIST_29(op, arg, ...)  op(arg) MARCO_EXPAND(MAKE_ARG_LIST_28(op, __VA_ARGS__))
#define MAKE_ARG_LIST_30(op, arg, ...)  op(arg) MARCO_EXPAND(MAKE_ARG_LIST_29(op, __VA_ARGS__))
#define MAKE_ARG_LIST_31(op, arg, ...)  op(arg) MARCO_EXPAND(MAKE_ARG_LIST_30(op, __VA_ARGS__))
#define MAKE_ARG_LIST_32(op, arg, ...)  op(arg) MARCO_EXPAND(MAKE_ARG_LIST_31(op, __VA_ARGS__))
#define MAKE_ARG_LIST_33(op, arg, ...)  op(arg) MARCO_EXPAND(MAKE_ARG_LIST_32(op, __VA_ARGS__))
#define MAKE_ARG_LIST_34(op, arg, ...)  op(arg) MARCO_EXPAND(MAKE_ARG_LIST_33(op, __VA_ARGS__))
#define MAKE_ARG_LIST_35(op, arg, ...)  op(arg) MARCO_EXPAND(MAKE_ARG_LIST_34(op, __VA_ARGS__))
#define MAKE_ARG_LIST_36(op, arg, ...)  op(arg) MARCO_EXPAND(MAKE_ARG_LIST_35(op, __VA_ARGS__))
#define MAKE_ARG_LIST_37(op, arg, ...)  op(arg) MARCO_EXPAND(MAKE_ARG_LIST_36(op, __VA_ARGS__))
#define MAKE_ARG_LIST_38(op, arg, ...)  op(arg) MARCO_EXPAND(MAKE_ARG_LIST_37(op, __VA_ARGS__))
#define MAKE_ARG_LIST_39(op, arg, ...)  op(arg) MARCO_EXPAND(MAKE_ARG_LIST_38(op, __VA_ARGS__))
#define MAKE_ARG_LIST_40(op, arg, ...)  op(arg) MARCO_EXPAND(MAKE_ARG_LIST_39(op, __VA_ARGS__))
#define MAKE_ARG_LIST_41(op, arg, ...)  op(arg) MARCO_EXPAND(MAKE_ARG_LIST_40(op, __VA_ARGS__))
#define MAKE_ARG_LIST_42(op, arg, ...)  op(arg) MARCO_EXPAND(MAKE_ARG_LIST_41(op, __VA_ARGS__))
#define MAKE_ARG_LIST_43(op, arg, ...)  op(arg) MARCO_EXPAND(MAKE_ARG_LIST_42(op, __VA_ARGS__))
#define MAKE_ARG_LIST_44(op, arg, ...)  op(arg) MARCO_EXPAND(MAKE_ARG_LIST_43(op, __VA_ARGS__))
#define MAKE_ARG_LIST_45(op, arg, ...)  op(arg) MARCO_EXPAND(MAKE_ARG_LIST_44(op, __VA_ARGS__))
#define MAKE_ARG_LIST_46(op, arg, ...)  op(arg) MARCO_EXPAND(MAKE_ARG_LIST_45(op, __VA_ARGS__))
#define MAKE_ARG_LIST_47(op, arg, ...)  op(arg) MARCO_EXPAND(MAKE_ARG_LIST_46(op, __VA_ARGS__))
#define MAKE_ARG_LIST_48(op, arg, ...)  op(arg) MARCO_EXPAND(MAKE_ARG_LIST_47(op, __VA_ARGS__))
#define MAKE_ARG_LIST_49(op, arg, ...)  op(arg) MARCO_EXPAND(MAKE_ARG_LIST_48(op, __VA_ARGS__))
#define MAKE_ARG_LIST_50(op, arg, ...)  op(arg) MARCO_EXPAND(MAKE_ARG_LIST_49(op, __VA_ARGS__))
#define MAKE_ARG_LIST_51(op, arg, ...)  op(arg) MARCO_EXPAND(MAKE_ARG_LIST_50(op, __VA_ARGS__))
#define MAKE_ARG_LIST_52(op, arg, ...)  op(arg) MARCO_EXPAND(MAKE_ARG_LIST_51(op, __VA_ARGS__))
#define MAKE_ARG_LIST_53(op, arg, ...)  op(arg) MARCO_EXPAND(MAKE_ARG_LIST_52(op, __VA_ARGS__))
#define MAKE_ARG_LIST_54(op, arg, ...)  op(arg) MARCO_EXPAND(MAKE_ARG_LIST_53(op, __VA_ARGS__))
#define MAKE_ARG_LIST_55(op, arg, ...)  op(arg) MARCO_EXPAND(MAKE_ARG_LIST_54(op, __VA_ARGS__))
#define MAKE_ARG_LIST_56(op, arg, ...)  op(arg) MARCO_EXPAND(MAKE_ARG_LIST_55(op, __VA_ARGS__))
#define MAKE_ARG_LIST_57(op, arg, ...)  op(arg) MARCO_EXPAND(MAKE_ARG_LIST_56(op, __VA_ARGS__))
#define MAKE_ARG_LIST_58(op, arg, ...)  op(arg) MARCO_EXPAND(MAKE_ARG_LIST_57(op, __VA_ARGS__))
#define MAKE_ARG_LIST_59(op, arg, ...)  op(arg) MARCO_EXPAND(MAKE_ARG_LIST_58(op, __VA_ARGS__))
#define MAKE_ARG_LIST_60(op, arg, ...)  op(arg) MARCO_EXPAND(MAKE_ARG_LIST_59(op, __VA_ARGS__))

#define GEN_ENCODE_OBJECT(...) MARCO_EXPAND(__VA_ARGS__)
#define GEN_DECODE_OBJECT(...) MARCO_EXPAND(__VA_ARGS__)

#define MACRO_CONCAT(A, B)  A##_##B

#define MAKE_ARG_LIST(N, op, arg, ...) \
    MACRO_CONCAT(MAKE_ARG_LIST, N)(op, arg, __VA_ARGS__)

#define GEN_ENCODE_FUNC(N, ...) \
    GEN_ENCODE_OBJECT(MAKE_ARG_LIST(N, ENCODE_FIELD, __VA_ARGS__))

#define GEN_DECODE_FUNC(N, ...) \
    GEN_DECODE_OBJECT(MAKE_ARG_LIST(N, DECODE_FIELD, __VA_ARGS__))

#define RSEQ_N() \
    119, 118, 117, 116, 115, 114, 113, 112, 111, 110, \
    109, 108, 107, 106, 105, 104, 103, 102, 101, 100, \
    99, 98, 97, 96, 95, 94, 93, 92, 91, 90, \
    89, 88, 87, 86, 85, 84, 83, 82, 81, 80, \
    79, 78, 77, 76, 75, 74, 73, 72, 71, 70, \
    69, 68, 67, 66, 65, 64, 63, 62, 61, 60, \
    59, 58, 57, 56, 55, 54, 53, 52, 51, 50, \
    49, 48, 47, 46, 45, 44, 43, 42, 41, 40, \
    39, 38, 37, 36, 35, 34, 33, 32, 31, 30, \
    29, 28, 27, 26, 25, 24, 23, 22, 21, 20, \
    19, 18, 17, 16, 15, 14, 13, 12, 11, 10, \
    9, 8, 7, 6, 5, 4, 3, 2, 1, 0

#define ARG_N( \
    _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, \
    _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, \
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, \
    _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, \
    _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, \
    _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, \
    _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, \
    _71, _72, _73, _74, _75, _76, _77, _78, _79, _80, \
    _81, _82, _83, _84, _85, _86, _87, _88, _89, _90, \
    _91, _92, _93, _94, _95, _96, _97, _98, _99, _100, \
    _101, _102, _103, _104, _105, _106, _107, _108, _109, _110, \
    _111, _112, _113, _114, _115, _116, _117, _118, _119, N, ...) N

#define GET_ARG_COUNT_INNER(...)    MARCO_EXPAND(ARG_N(__VA_ARGS__))
#define GET_ARG_COUNT(...)          GET_ARG_COUNT_INNER(__VA_ARGS__, RSEQ_N())

#define JSON_ENCODE(...) GEN_ENCODE_FUNC(GET_ARG_COUNT(__VA_ARGS__), __VA_ARGS__)
#define JSON_DECODE(...) GEN_DECODE_FUNC(GET_ARG_COUNT(__VA_ARGS__), __VA_ARGS__)

#define  SHINE_JSON_ENCODE(...) SHINE_JSON_ENCODE(__VA_ARGS__)

#define SHINE_JSON_MODEL(TYPE, ...) shine::string encode(){\
    shine::string ret = "{"; \
    bool empty = true; \
    JSON_ENCODE(__VA_ARGS__); \
    ret += "}"; \
    return ret; \
    }\
    \
    bool decode(const shine::string &str){\
    shine::json json;\
    if (!json.decode(str))  return false;\
    shine::json_node_t *node = &json.get_root();\
    JSON_DECODE(__VA_ARGS__); \
    return true; \
    } \
    \
    bool decode(shine::json_node_t *node){\
    JSON_DECODE(__VA_ARGS__); \
    return true; \
    }}; \
    inline shine::string encode_field(TYPE &val){return val.encode();}\
    inline void decode_field(TYPE &val, shine::json_node_t *node){\
        val.decode(node);

namespace shine
{
    class json_node_t{
        friend class json;
        friend class convert;
    public:
        typedef std::shared_ptr<std::map<string, json_node_t> > kv_childs_t;
        typedef std::shared_ptr<std::deque<json_node_t> > array_childs_t;

        enum {
            e_string = 0,
            e_integer = 1,
            e_float = 2,
            e_boolean = 3,
            e_null = 4,
            e_object = 5,
            e_array = 6,
            e_root = 7,
        };

        enum {
            e_decode_key = 0,
            e_decode_type = 1,
            e_decode_value = 2,
            e_decode_end = 3,
        };

    private:
        static inline bool skip_space(const string &data, size_t &pos){
            while (pos < data.size()
                && (data[pos] == '\r' || data[pos] == '\n'
                || data[pos] == '\t' || data[pos] == ' '))
            {
                ++pos;
            }

            return pos < data.size();
        }

        static inline bool skip_number(const string &data, size_t &pos){
            while (pos < data.size()
                && ((data[pos] >= '0' && data[pos] <= '9') || data[pos] == '.' || data[pos] == '-'))
            {
                ++pos;
            }

            return pos < data.size();
        }

        static inline bool find(int8 ch, const string &data, size_t &pos){
            while (pos < data.size() && data[pos] != ch)
            {
                ++pos;
            }

            return pos < data.size();
        }

#define SKIP_SPACE() if (!skip_space(data, cost_len)) return false;

        static bool parse_number(json_node_t &node, const string &data, size_t &cost_len)
        {
            size_t value_begin = cost_len++;
            if (!skip_number(data, cost_len))
                return false;

            node.get_value().assign(data.data() + value_begin, cost_len - value_begin);

            string &value = node.get_value();
            auto tmp = value.find_first_of('.');
            if (tmp != value.find_last_of('.') 
                || value[value.size() - 1] == '.' 
                || (value.find_first_of('-') != 0 && value.find_first_of('-') != string::npos)
                || (value.size() >= 2 && value[0] == '-' && value[1] == '.')
                )
                return false;

            node.set_type(tmp == string::npos ? e_integer : e_float);
                return true;

        }

        static bool parse_string(json_node_t &node, const string &data, size_t &cost_len)
        {
            int8 ch = data[cost_len++];
            size_t value_begin = cost_len;

            for (;;)
            {
                if (!find(ch, data, cost_len))
                    return false;

                if (data[cost_len - 1] != '\\')
                    break;
                else
                {
                    ++cost_len;
                    continue;
                }

            }

            node.get_value().assign(data.data() + value_begin, cost_len - value_begin);
            node.set_type(e_string);
            node.set_decode_step(e_decode_end);

            ++cost_len;

            return true;

        }

        static bool parse_symbol(json_node_t &node, const string &data, size_t &cost_len, uint8 type, const string &symbol)
        {
            cost_len += symbol.size();
            SKIP_SPACE();

            if (data[cost_len] != ',' && data[cost_len] != '}')
                return false;

            node.set_type(type);
            node.get_value() = symbol;
            node.set_decode_step(e_decode_end);

            return true;

        }

        static bool parse_object(json_node_t &node, const string &data, size_t &cost_len, uint8 decode_step, int8 symbol)
        {
            for (;;)
            {
                json_node_t sub_node;
                sub_node.set_decode_step(decode_step);
                if (!decode(sub_node, data, cost_len))
                    return false;

                SKIP_SPACE();

                if (sub_node.get_key().empty())
                {
                    if (!node.get_array_childs())
                        node.get_array_childs() = std::make_shared<std::deque<json_node_t>>();
                    node.get_array_childs()->push_back(std::move(sub_node));
                }
                else
                {
                    if (!node.get_kv_childs())
                        node.get_kv_childs() = std::make_shared<std::map<string, json_node_t>>();

                    node.get_kv_childs()->emplace(sub_node.get_key(), std::move(sub_node));
                }

                if (data[cost_len] == symbol)
                {
                    ++cost_len;
                    node.set_decode_step(e_decode_end);
                    return true;
                }
                else if (data[cost_len] == ',')
                {
                    ++cost_len;
                    continue;
                }
                else
                {
                    return false;
                }
            }
        }

        static bool decode(json_node_t& node, const string &data, size_t &cost_len){

            if (node.get_decode_step() == e_decode_key)
            {
                SKIP_SPACE();

                int8 ch = data[cost_len];
                if (ch == '\'' || ch == '"')
                {
                    size_t key_begin = ++cost_len;
                    
                    for (;;)
                    {
                        if (!find(ch, data, cost_len))
                            return false;

                        if (data[cost_len - 1] != '\\')
                            break;
                        else
                            ++cost_len;
                    }

                    node.get_key().assign(data.data() + key_begin, cost_len - key_begin);
                    node.set_decode_step(e_decode_type);
                }
                else if (ch == '}')
                {
                    return true;
                }
                else
                {
                    return false;
                }
            }

            if (node.get_decode_step() == e_decode_type)
            {
                SKIP_SPACE();

                if (!node.get_key().empty())
                {
                    if (data[++cost_len] != ':')
                        return false;

                    ++cost_len;
                    SKIP_SPACE();
                }

                int8 ch = data[cost_len];
                if (ch == '{')
                {
                    ++cost_len;
                    node.set_type(e_object);
                    node.set_decode_step(e_decode_value);
                }
                else if (ch == '[')
                {
                    ++cost_len;
                    node.set_type(e_array);
                    node.set_decode_step(e_decode_value);
                }
                else if ((ch >= '0' && ch <= '9') || (ch == '-' && data[cost_len + 1] >= '0' && data[cost_len + 1] <= '9'))
                {
                    return parse_number(node, data, cost_len);
                }
                else if (ch == '"' || ch == '\'')
                {
                    return parse_string(node, data, cost_len);
                }
                else if (memcmp(data.data() + cost_len, "true", 4) == 0)
                {
                    return parse_symbol(node, data, cost_len, e_boolean, "true");
                }
                else if (memcmp(data.data() + cost_len, "false", 5) == 0)
                {
                    return parse_symbol(node, data, cost_len, e_boolean, "false");
                }
                else if (memcmp(data.data() + cost_len, "null", 4) == 0)
                {
                    return parse_symbol(node, data, cost_len, e_null, "null");
                }
            }

            if (node.get_decode_step() == e_decode_value)
            {
                if (node.get_type() == e_object)
                {
                    return parse_object(node, data, cost_len, e_decode_key, '}');
                }
                else if (node.get_type() == e_array)
                {
                    return parse_object(node, data, cost_len, e_decode_type, ']');
                }
            }
            return true;
        }

        public:
        static void encode_string(string &out, const string &in)
        {
            for (size_t i = 0; i < in.size(); i++)
            {
                switch (in[i])
                {
            		case '\\':
                        out += "\\\\";
                        break;
                    case '"':
                        out += "\\\"";
                        break;
//                     case '/':
//                         out += "\\/";
//                         break;
                    case '\b':
                        out += "\\b";
                        break;
                    case '\f':
                        out += "\\f";
                        break;
                    case '\t':
                        out += "\\t";
                        break;
                    case '\n':
                        out += "\\n";
                        break;
                    case '\r':
                        out += "\\r";
                        break;
                    default:
                        out += in[i];
                        break;
                }

            }
        }

        private:
        static void encode(json_node_t &node, string &data)
        {
            if (!node.get_key().empty())
            {
                data += "\"";
                encode_string(data, node.get_key());
                data += "\":";
            }

            if (node.get_type() == e_string)
            {
                data += "\"";
                encode_string(data, node.get_value());
                data += "\"";
            }
            else if (node.get_type() == e_object)
            {
                data += "{";

                if (node.get_kv_childs())
                {
                    bool flag = false;
                    for (auto iter : *node.get_kv_childs())
                    {
                        if (flag)
                            data += ",";

                        encode(iter.second, data);                     
                        flag = true;
                    }
                }

                data += "}";
            }
            else if (node.get_type() == e_array)
            {
                data += "[";

                if (node.get_array_childs())
                {
                    bool flag = false;
                    for (auto &iter : *node.get_array_childs())
                    {
                        if (flag)
                            data += ",";

                        encode(iter, data);
                        flag = true;
                    }
                }

                data += "]";
            }
            else
            {
                data +=  node.get_value();
            }
        }

    public:
#define SET_TYPE(type) \
        if (get_type() != type)\
        {\
            set_type(type); \
            get_kv_childs().reset(); \
            get_array_childs().reset(); \
            get_value().clear(); \
        }

        json_node_t &clear()
        {
            set_null();
            return *this;
        }

        json_node_t &set_null()
        {
            SET_TYPE(e_null);
            set_value("null");
            return *this;
        }

        json_node_t &set_string(const string &value)
        {
            SET_TYPE(e_string);

            set_value(value);
            return *this;

        }

        json_node_t &set_boolean(bool value)
        {
            SET_TYPE(e_boolean);

            set_value(value ? "true" : "false");
            return *this;
        }

        json_node_t &set_number(const string &value)
        {
            string tmp = value;
            tmp += '\0';
            size_t cost_len = 0;
            if (parse_number(*this, tmp, cost_len))
            {
                get_kv_childs().reset();
                get_array_childs().reset();
            }

            return *this;
        }

        size_t get_kv_childs_size()
        {
            if (get_kv_childs())
                return get_kv_childs()->size();

            return 0;
        }

        void foreach_kv_childs(std::function<void(const string &, const json_node_t &)> func)
        {
            if (get_kv_childs())
            {
                for (auto iter : *get_kv_childs())
                    func(iter.first, iter.second);
            }
        }

        json_node_t &insert_kv_child(json_node_t &node)
        {
            SET_TYPE(e_object);

            if (!node.get_key().empty())
            {
                if (!get_kv_childs())
                    get_kv_childs() = std::make_shared<std::map<string, json_node_t>>();

                get_kv_childs()->emplace(node.get_key(), node);
            }

            return *this;
        }

        json_node_t &erase_kv_child(const string &key)
        {
            SET_TYPE(e_object);

            if (get_kv_childs())
                get_kv_childs()->erase(key);

            return *this;
        }

        json_node_t *find_kv_child(const string &key){
            if (get_type() != e_object)
                return nullptr;

            if (get_kv_childs())
            {
                auto iter = get_kv_childs()->find(key);
                if (iter != get_kv_childs()->end())
                    return &iter->second;
            }

            return nullptr;
        }

        json_node_t &clear_kv_childs(const string &key)
        {
            SET_TYPE(e_object);

            get_kv_childs().reset();

            return *this;
        }

        size_t get_array_childs_size()
        {
            if (get_array_childs())
                return get_array_childs()->size();

            return 0;
        }

        void foreach_array_childs(std::function<void(const size_type, const json_node_t &)> func)
        {
            if (get_array_childs())
            {
                std::deque<json_node_t> &arr = *get_array_childs();
                for (size_type i = 0; i < arr.size(); i++)
                {
                    func(i, arr[i]);
                }
            }
        }

        json_node_t &push_back_array_child(json_node_t &node)
        {
            SET_TYPE(e_array);

            if (!get_array_childs())
                get_array_childs() = std::make_shared<std::deque<json_node_t>>();

            get_array_childs()->push_back(node);

            return *this;
        }

        json_node_t &push_front_array_child(json_node_t &node)
        {
            SET_TYPE(e_array);

            if (!get_array_childs())
                get_array_childs() = std::make_shared<std::deque<json_node_t>>();

            get_array_childs()->push_front(node);

            return *this;
        }

        json_node_t &erase_array_child(size_type index)
        {
            SET_TYPE(e_array);

            if (get_array_childs() && get_array_childs()->size() > index)
                    get_array_childs()->erase(get_array_childs()->begin() + index);

            return *this;
        }

        json_node_t *get_array_child(size_type index)
        {
            if (get_array_childs() && get_array_childs()->size() > index)
                    return &(*get_array_childs())[index];

            return nullptr;
        }

        json_node_t &clear_array_childs()
        {
            SET_TYPE(e_array);

            get_array_childs().reset();

            return *this;
        }

    private:
        SHINE_GEN_MEMBER_GETSET(uint8, decode_step);
        SHINE_GEN_MEMBER_GETSET(uint8, type, = e_null);
        SHINE_GEN_MEMBER_GETSET(string, key);
        SHINE_GEN_MEMBER_GETSET(string, value);
        SHINE_GEN_MEMBER_GETSET(kv_childs_t, kv_childs);
        SHINE_GEN_MEMBER_GETSET(array_childs_t, array_childs);
    };

    SHINE_CHECK_MEMBER(encode);

    class json
    {
    public:
        json(){
            get_root().set_decode_step(json_node_t::e_decode_type);
        }

        /** 
         *@brief 将json对象编码成json字符串
         *@return shine::string 
         *@warning 
         *@note 
        */
        string encode(){
            string str;
            get_root().get_key().clear();
            json_node_t::encode(get_root(), str);
            return std::move(str);
        }

        /** 
         *@brief 将json字符串解码成json对象
         *@param data 
         *@return bool 
         *@warning 
         *@note 
        */
        bool decode(const string &data){
            size_t cost_len = 0;
            return json_node_t::decode(get_root(), data, cost_len);
        }

        SHINE_GEN_MEMBER_GETSET(json_node_t, root);
    };
}


#define ENCODE_FIELD(field) \
{ \
    shine::string tmp = ::encode_field(field); \
if (!tmp.empty())\
    {\
if (!empty) ret += ","; \
    ret += "\""; \
    ret += #field; \
    ret += "\":"; \
    ret += ::encode_field(field); \
    empty = false; \
}\
};

#define DECODE_FIELD(field) {\
    shine::json_node_t *node2 = node->find_kv_child(#field);\
    if (node2) ::decode_field(field, node2);\
}

inline shine::string encode_field(bool val){
    shine::string ret = val ? "true" : "false";
    return ret;
}

inline void decode_field(bool &val, shine::json_node_t *node){
    val = node->get_value() == "true";
}

#define ENCODE_STRING_FIELD(TYPE) \
    inline shine::string encode_field(const TYPE &val){\
        shine::string ret; \
        shine::json_node_t::encode_string(ret, val); \
        ret.insert(0, "\""); \
        ret += '\"'; \
        return ret; \
    }

ENCODE_STRING_FIELD(shine::string);
inline void decode_field(shine::string &val, shine::json_node_t *node){
    val = node->get_value();
}

ENCODE_STRING_FIELD(std::string);
inline void decode_field(std::string &val, shine::json_node_t *node){
    val = node->get_value();
}

inline shine::string encode_field(const shine::int8 &val){
        shine::string ret; 
        ret += "\"";
        ret += val;
        ret += '\"'; 
        return ret; 
}

inline void decode_field(shine::int8 &val, shine::json_node_t *node){
        val = node->get_value()[0];
}

ENCODE_STRING_FIELD(shine::int8*);
inline void decode_field(shine::int8 *&val, shine::json_node_t *node){
    val = (shine::int8 *)node->get_value().c_str();
}

#define ENCODE_NUMERIC_FIELD(TYPE) \
    inline shine::string encode_field(const TYPE &val){\
        return shine::string(val); \
    }

ENCODE_NUMERIC_FIELD(shine::int16);
inline void decode_field(shine::int16 &val, shine::json_node_t *node){
    val = (shine::int16)std::stol(node->get_value());
}

ENCODE_NUMERIC_FIELD(shine::uint16);
inline void decode_field(shine::uint16 &val, shine::json_node_t *node){
    val = (shine::uint16)std::stoul(node->get_value());
}

ENCODE_NUMERIC_FIELD(shine::int32);
inline void decode_field(shine::int32 &val, shine::json_node_t *node){
    val = std::stol(node->get_value());
}

ENCODE_NUMERIC_FIELD(shine::uint32);
inline void decode_field(shine::uint32 &val, shine::json_node_t *node){
    val = std::stoul(node->get_value());
}

ENCODE_NUMERIC_FIELD(shine::Long);
inline void decode_field(shine::Long &val, shine::json_node_t *node){
    val = std::stol(node->get_value());
}

ENCODE_NUMERIC_FIELD(shine::uLong);
inline void decode_field(shine::uLong &val, shine::json_node_t *node){
    val = std::stoul(node->get_value());
}

ENCODE_NUMERIC_FIELD(shine::int64);
inline void decode_field(shine::int64 &val, shine::json_node_t *node){
    val = std::stoll(node->get_value());
}

ENCODE_NUMERIC_FIELD(shine::uint64);
inline void decode_field(shine::uint64 &val, shine::json_node_t *node){
    val = std::stoull(node->get_value());
}

ENCODE_NUMERIC_FIELD(float);
inline void decode_field(float &val, shine::json_node_t *node){
    val = std::stof(node->get_value());
}

ENCODE_NUMERIC_FIELD(double);
inline void decode_field(double &val, shine::json_node_t *node){
    val = std::stod(node->get_value());
}

ENCODE_NUMERIC_FIELD(long double);
inline void decode_field(long double &val, shine::json_node_t *node){
    val = std::stold(node->get_value());
}

#define ENCODE_MAP_FIELD(TYPE) \
    template<typename T1, typename T2>\
    inline shine::string encode_field(TYPE<T1, T2> &val){\
if (val.empty()) return ""; \
    shine::string ret = "{"; \
    bool empty = true; \
for (auto &iter : val)\
{\
    shine::string k = "\"" + encode_field(iter.first) + "\""; \
    shine::string v = encode_field(iter.second); \
if (!v.empty())\
{\
if (!empty)\
    ret += ","; \
    \
    ret += k; \
    ret += ":"; \
    ret += v; \
    empty = false; \
}\
}\
    \
    ret += "}"; \
    \
    return ret; \
    }


#define DECODE_MAP_FIELD(TYPE) \
    template<typename T1, typename T2>\
    inline void decode_field(TYPE<T1, T2> &val, shine::json_node_t *node){\
    node->foreach_kv_childs([&val](const shine::string &key, const shine::json_node_t &value){\
        shine::json_node_t tmp;\
        tmp.set_string(key);\
        T1 k;\
        T2 v;\
        decode_field(k, (shine::json_node_t *)&tmp); \
        decode_field(v, (shine::json_node_t *)&value); \
        val.emplace(std::move(k), std::move(v));\
    }); \
    }


ENCODE_MAP_FIELD(std::map);
DECODE_MAP_FIELD(std::map);

ENCODE_MAP_FIELD(std::unordered_map);
DECODE_MAP_FIELD(std::unordered_map);

#define ENCODE_ARRAY_FIELD(TYPE) \
    template<typename T>\
    inline shine::string encode_field(TYPE<T> &val){\
if (val.empty()) return ""; \
    shine::string ret = "["; \
    bool empty = true; \
for (auto &iter : val)\
{\
    shine::string v = encode_field(iter); \
if (!v.empty())\
{\
if (!empty)\
    ret += ","; \
    \
    ret += v; \
    empty = false; \
}\
}\
    \
    ret += "]"; \
    \
    return ret; \
    }


#define DECODE_ARRAY_FIELD(TYPE) \
    template<typename T>\
    inline void decode_field(TYPE<T> &val, shine::json_node_t *node){\
    node->foreach_array_childs([&val](const shine::size_type, const shine::json_node_t &value){\
    T v;\
    ::decode_field(v, (shine::json_node_t *)&value); \
    val.emplace_back(std::move(v)); \
    }); \
    }

ENCODE_ARRAY_FIELD(std::vector);
DECODE_ARRAY_FIELD(std::vector);

ENCODE_ARRAY_FIELD(std::deque);
DECODE_ARRAY_FIELD(std::deque);

ENCODE_ARRAY_FIELD(std::list);
DECODE_ARRAY_FIELD(std::list);

ENCODE_ARRAY_FIELD(std::forward_list);

template<typename T>
inline void decode_field(std::forward_list<T> &val, shine::json_node_t *node){
        node->foreach_array_childs([&val](const shine::size_type, const shine::json_node_t &value){
        T v; 
        ::decode_field(v, (shine::json_node_t *)&value);
        val.emplace_front(std::move(v));
    }); 
}

#define ENCODE_SET_FIELD(TYPE) ENCODE_ARRAY_FIELD(TYPE)


#define DECODE_SET_FIELD(TYPE) \
    template<typename T>\
    inline void decode_field(TYPE<T> &val, shine::json_node_t *node){\
    node->foreach_array_childs([&val](const shine::size_type, const shine::json_node_t &value){\
    T v;\
    ::decode_field(v, (shine::json_node_t *)&value); \
    val.emplace(std::move(v)); \
    }); \
    }

ENCODE_SET_FIELD(std::set);
DECODE_SET_FIELD(std::set);

ENCODE_SET_FIELD(std::unordered_set);
DECODE_SET_FIELD(std::unordered_set);

