

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

#define JSON_GEN_ENCODE_FUNC(N, ...) \
    MARCO_EXPAND_WARP(MAKE_ARG_LIST(N, JSON_ENCODE_FIELD, __VA_ARGS__))
#define JSON_GEN_FORMAT_ENCODE_FUNC(N, ...) \
    MARCO_EXPAND_WARP(MAKE_ARG_LIST(N, JSON_FORMAT_ENCODE_FIELD, __VA_ARGS__))

#define JSON_GEN_DECODE_FUNC(N, ...) \
    MARCO_EXPAND_WARP(MAKE_ARG_LIST(N, JSON_DECODE_FIELD, __VA_ARGS__))

#define JSON_ENCODE(...) JSON_GEN_ENCODE_FUNC(GET_ARG_COUNT(__VA_ARGS__), __VA_ARGS__)
#define JSON_FORMAT_ENCODE(...) JSON_GEN_FORMAT_ENCODE_FUNC(GET_ARG_COUNT(__VA_ARGS__), __VA_ARGS__)
#define JSON_DECODE(...) JSON_GEN_DECODE_FUNC(GET_ARG_COUNT(__VA_ARGS__), __VA_ARGS__)

#define SHINE_JSON_MODEL(TYPE, ...) shine::string json_encode() const{\
    shine::string ret; \
    bool empty = true; \
    JSON_ENCODE(__VA_ARGS__); \
if (!empty){ ret.insert(0, "{"); ret += "}"; }\
    return ret; \
    }\
    \
    bool json_decode(const shine::string &str){\
    shine::json json;\
    if (!json.decode(str))  return false;\
    shine::json_node_t *node = &json.get_root();\
    JSON_DECODE(__VA_ARGS__); \
    return true; \
    } \
    \
    bool json_decode(shine::json_node_t *node){\
    JSON_DECODE(__VA_ARGS__); \
    return true; \
    }}; \
    inline shine::string json_encode_field(const TYPE &val){ return val.json_encode(); }\
    inline void json_decode_field(TYPE &val, shine::json_node_t *node){\
    val.json_decode(node);

namespace shine
{
	class json_node_t {
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
		static inline bool skip_space(const string &data, size_t &pos) {
			while (pos < data.size()
				&& (data[pos] == '\r' || data[pos] == '\n'
					|| data[pos] == '\t' || data[pos] == ' '))
			{
				++pos;
			}

			return pos < data.size();
		}

		static inline bool skip_number(const string &data, size_t &pos) {
			while (pos < data.size()
				&& ((data[pos] >= '0' && data[pos] <= '9') || data[pos] == '.' || data[pos] == '-'))
			{
				++pos;
			}

			return pos < data.size();
		}

		static inline bool find(int8 ch, const string &data, size_t &pos) {
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

		static inline string format(const string &data, int indent = 0) {
			size_t cost_len = 0;
			json_node_t node;
			if (!decode(node, data, cost_len))
				return "";
			

			string ret;
			format(node, ret, 0, true);
			return std::move(ret);
		}

		static void format(const json_node_t &node, string &data, size_t indent, bool first) {
			if (node.get_type() == e_string)
			{
				data.append(indent, ' ');
				data.append("\"").append(node.get_key()).append("\": \"").append(node.get_value()).append("\"");
				if (!first) data.append("\n");
			}
			else if (node.get_type() == e_integer || node.get_type() == e_float 
				|| node.get_type() == e_boolean || node.get_type() == e_null)
			{
				data.append(indent, ' ');
				data.append("\"").append(node.get_key()).append("\": ").append(node.get_value()).append("");
				if (!first) data.append("\n");
			}
			else if (node.get_type() == e_object)
			{
				data.append(indent, ' ');
				data.append("\"").append(node.get_key()).append("\": {\n");
				bool sub_first = true;
				node.foreach_kv_childs([&data, &sub_first, indent](const string &key, const json_node_t &val) {
					format(val, data, indent + 4, sub_first);
					sub_first = false;
				});
				data.append(indent, ' ');
				data.append("}");
				if (!first) data.append("\n");
			}
			else if (node.get_type() == e_array)
			{
				data.append(indent, ' ');
				data.append("\"").append(node.get_key()).append("\": [\n");
				bool sub_first = true;
				node.foreach_kv_childs([&data, &sub_first, indent](const string &key, const json_node_t &val) {
					format(val, data, indent + 4, sub_first);
					sub_first = false;
				});
				data.append(indent, ' ');
				data.append("]");
				if (!first) data.append("\n");
			}
		}

		static bool decode(json_node_t& node, const string &data, size_t &cost_len) {

			if (node.get_decode_step() == e_decode_key)
			{
				SKIP_SPACE();

				int8 ch = data[cost_len];
				if (ch == '\'' || ch == '"')
				{
					size_t key_begin = ++cost_len;

					if (!find(ch, data, cost_len))
						return false;

					node.get_key().assign(data.data() + key_begin, cost_len - key_begin);
					++cost_len;
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
					if (data[cost_len++] != ':')
						return false;
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
				data += node.get_value();
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

		void foreach_kv_childs(std::function<void(const string &, const json_node_t &)> func) const
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

		json_node_t *find_kv_child(const string &key) {
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

		size_t get_array_childs_size() const
		{
			if (get_array_childs())
				return get_array_childs()->size();

			return 0;
		}

		void foreach_array_childs(std::function<void(const size_type, const json_node_t &)> func) const
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

		json_node_t *get_array_child(size_type index) const
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
		SHINE_GEN_MEMBER_GETSET(uint8, type, = e_object);
		SHINE_GEN_MEMBER_GETSET(string, key);
		SHINE_GEN_MEMBER_GETSET(string, value);
		SHINE_GEN_MEMBER_GETSET(kv_childs_t, kv_childs);
		SHINE_GEN_MEMBER_GETSET(array_childs_t, array_childs);
	};

	SHINE_CHECK_MEMBER(encode);

	class json
	{
	public:
		json() {
			get_root().set_decode_step(json_node_t::e_decode_type);
		}

		static inline string format(const string &data) {
			json obj;
			if (!obj.decode(data))
				return "";

			string ret;
			bool first = true;
			format(obj.get_root(), ret, 0, first);
			return std::move(ret);
		}

		static void format(const json_node_t &node, string &data, size_t indent, bool &first) {
			if (node.get_type() == json_node_t::e_string)
			{
				if (!first) data.append(",\n");
				data.append(indent, ' ');
				if (!node.get_key().empty())
				{
					data.append("\"").append(node.get_key()).append("\": ");
				}

				data.append("\"").append(node.get_value()).append("\"");
			}
			else if (node.get_type() == json_node_t::e_integer || node.get_type() == json_node_t::e_float
				|| node.get_type() == json_node_t::e_boolean || node.get_type() == json_node_t::e_null)
			{
				if (!first) data.append(",\n");

				data.append(indent, ' ');
				if (!node.get_key().empty())
				{
					data.append("\"").append(node.get_key()).append("\": ");
				}

				data.append(node.get_value());
			}
			else if (node.get_type() == json_node_t::e_object)
			{
				if (!first) data.append(",\n");

				data.append(indent, ' ');
				if (!node.get_key().empty())
				{
					data.append("\"").append(node.get_key()).append("\": ");
				}

				data.append("{\n");
				bool sub_first = true;
				node.foreach_kv_childs([&data, &sub_first, indent](const string &key, const json_node_t &val) {
					format(val, data, indent + 4, sub_first);
					sub_first = false;
				});
				data.append("\n");
				data.append(indent, ' ');
				data.append("}");
			}
			else if (node.get_type() == json_node_t::e_array)
			{
				if (!first) data.append(",\n");

				data.append(indent, ' ');
				if (!node.get_key().empty())
				{
					data.append("\"").append(node.get_key()).append("\": ");
				}
				data.append("[\n");
				bool sub_first = true;
				node.foreach_array_childs([&data, &sub_first, indent](const size_t index, const json_node_t &val) {
					format(val, data, indent + 4, sub_first);
					sub_first = false;
				});
				data.append("\n");
				data.append(indent, ' ');
				data.append("]");
			}
		}

		inline string format() {
			return format(encode());
		}

		/**
		*@brief 将json对象编码成json字符串
		*@return shine::string
		*@warning
		*@note
		*/
		inline string encode() {
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
		bool decode(const string &data) {
			size_t cost_len = 0;
			return json_node_t::decode(get_root(), data, cost_len);
		}

		SHINE_GEN_MEMBER_GETSET(json_node_t, root);
	};
}


#define JSON_ENCODE_FIELD(field) \
{ \
    shine::string tmp = ::json_encode_field(this->field); \
if (!tmp.empty())\
    {\
if (!empty) ret += ","; \
    ret += "\""; \
    ret += #field; \
    ret += "\":"; \
    ret += tmp; \
    empty = false; \
}\
};

#define JSON_FORMAT_ENCODE_FIELD(field) \
{ \
    shine::string tmp = ::json_encode_field(this->field); \
if (!tmp.empty())\
    {\
if (!empty) ret += ","; \
    ret += "\""; \
    ret += #field; \
    ret += "\":"; \
    ret += tmp; \
    empty = false; \
}\
};

#define JSON_DECODE_FIELD(field) {\
    shine::json_node_t *node2 = node->find_kv_child(#field);\
    if (node2) ::json_decode_field(this->field, node2); \
}

inline shine::string json_encode_field(const bool val) {
	shine::string ret = val ? "true" : "false";
	return ret;
}

inline void json_decode_field(bool &val, shine::json_node_t *node) {
	val = node->get_value() == "true";
}

#define JSON_ENCODE_STRING_FIELD(TYPE) \
    inline shine::string json_encode_field(const TYPE &val){\
        shine::string ret; \
        shine::json_node_t::encode_string(ret, val); \
        if (!ret.empty()){\
        ret.insert(0, "\""); \
        ret += '\"'; }\
        return ret; \
    }

JSON_ENCODE_STRING_FIELD(shine::string);
inline void json_decode_field(shine::string &val, shine::json_node_t *node) {
	val = node->get_value();
}

JSON_ENCODE_STRING_FIELD(std::string);
inline void json_decode_field(std::string &val, shine::json_node_t *node) {
	val = node->get_value();
}

inline shine::string json_encode_field(const shine::int8 &val) {
	shine::string ret;
	ret += "\"";
	ret += val;
	ret += '\"';
	return ret;
}

inline void json_decode_field(shine::int8 &val, shine::json_node_t *node) {
	val = node->get_value()[0];
}

JSON_ENCODE_STRING_FIELD(shine::int8*);
inline void json_decode_field(shine::int8 *&val, shine::json_node_t *node) {
	val = (shine::int8 *)node->get_value().c_str();
}

#define JSON_ENCODE_NUMERIC_FIELD(TYPE) \
    inline shine::string json_encode_field(const TYPE &val){\
        return shine::string(val); \
    }

JSON_ENCODE_NUMERIC_FIELD(shine::int16);
inline void json_decode_field(shine::int16 &val, shine::json_node_t *node) {
	val = node->get_value().to_int16();
}

JSON_ENCODE_NUMERIC_FIELD(shine::uint16);
inline void json_decode_field(shine::uint16 &val, shine::json_node_t *node) {
	val = node->get_value().to_uint16();
}

JSON_ENCODE_NUMERIC_FIELD(shine::int32);
inline void json_decode_field(shine::int32 &val, shine::json_node_t *node) {
	val = node->get_value().to_int32();
}

JSON_ENCODE_NUMERIC_FIELD(shine::uint32);
inline void json_decode_field(shine::uint32 &val, shine::json_node_t *node) {
	val = node->get_value().to_uint32();
}

JSON_ENCODE_NUMERIC_FIELD(shine::Long);
inline void json_decode_field(shine::Long &val, shine::json_node_t *node) {
	val = node->get_value().to_long();
}

JSON_ENCODE_NUMERIC_FIELD(shine::uLong);
inline void json_decode_field(shine::uLong &val, shine::json_node_t *node) {
	val = node->get_value().to_ulong();
}

JSON_ENCODE_NUMERIC_FIELD(shine::int64);
inline void json_decode_field(shine::int64 &val, shine::json_node_t *node) {
	val = node->get_value().to_int64();
}

JSON_ENCODE_NUMERIC_FIELD(shine::uint64);
inline void json_decode_field(shine::uint64 &val, shine::json_node_t *node) {
	val = node->get_value().to_uint64();
}

JSON_ENCODE_NUMERIC_FIELD(shine::Float);
inline void json_decode_field(shine::Float &val, shine::json_node_t *node) {
	val = node->get_value().to_float();
}

JSON_ENCODE_NUMERIC_FIELD(shine::Double);
inline void json_decode_field(shine::Double &val, shine::json_node_t *node) {
	val = node->get_value().to_double();
}

JSON_ENCODE_NUMERIC_FIELD(shine::LDouble);
inline void json_decode_field(shine::LDouble &val, shine::json_node_t *node) {
	val = node->get_value().to_long_double();
}

#define JSON_ENCODE_MAP_FIELD(TYPE) \
    template<typename T1, typename T2>\
    inline shine::string json_encode_field(const TYPE<T1, T2> &val){\
if (val.empty()) return ""; \
    shine::string ret = "{"; \
    bool empty = true; \
for (auto &iter : val)\
{\
    shine::string k;\
    if (std::is_same<std::string, T1>::value) {k = json_encode_field(iter.first);} \
    else if (std::is_same<shine::string, T1>::value) { k = json_encode_field(iter.first); }\
    else { k = "\"" + json_encode_field(iter.first) + "\""; } \
    shine::string v = json_encode_field(iter.second); \
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


#define JSON_DECODE_MAP_FIELD(TYPE) \
    template<typename T1, typename T2>\
    inline void json_decode_field(TYPE<T1, T2> &val, shine::json_node_t *node){\
	val.clear();\
    node->foreach_kv_childs([&val](const shine::string &key, const shine::json_node_t &value){\
        shine::json_node_t tmp;\
        tmp.set_string(key);\
        T1 k;\
        T2 v;\
        json_decode_field(k, (shine::json_node_t *)&tmp); \
        json_decode_field(v, (shine::json_node_t *)&value); \
        val.emplace(std::move(k), std::move(v));\
    }); \
    }

JSON_ENCODE_MAP_FIELD(std::map);
JSON_DECODE_MAP_FIELD(std::map);

JSON_ENCODE_MAP_FIELD(std::unordered_map);
JSON_DECODE_MAP_FIELD(std::unordered_map);

#define JSON_ENCODE_MULTIMAP_FIELD(TYPE) \
    template<typename T1, typename T2>\
    inline shine::string json_encode_field(const TYPE<T1, T2> &val){\
if (val.empty()) return ""; \
    shine::string ret = "["; \
    bool empty = true; \
for (auto &iter : val)\
{\
    shine::string k;\
    if (std::is_same<std::string, T1>::value) {k = json_encode_field(iter.first);} \
    else if (std::is_same<shine::string, T1>::value) { k = json_encode_field(iter.first); }\
    else { k = "\"" + json_encode_field(iter.first) + "\""; } \
    shine::string v = json_encode_field(iter.second); \
	if (!v.empty())\
	{\
		if (!empty) \
			ret += ","; \
		ret += "{";\
		ret += k; \
		ret += ":"; \
		ret += v; \
		ret += "}";\
		empty = false; \
	}\
}\
    \
    ret += "]"; \
    \
    return ret; \
    }


#define JSON_DECODE_MULTIMAP_FIELD(TYPE) \
    template<typename T1, typename T2>\
    inline void json_decode_field(TYPE<T1, T2> &val, shine::json_node_t *node){\
	val.clear();\
    node->foreach_array_childs([&val](const shine::size_type, const shine::json_node_t &value){\
		    value.foreach_kv_childs([&val](const shine::string &key, const shine::json_node_t &value){\
			shine::json_node_t tmp;\
			tmp.set_string(key);\
			T1 k;\
			T2 v;\
			json_decode_field(k, (shine::json_node_t *)&tmp); \
			json_decode_field(v, (shine::json_node_t *)&value); \
			val.emplace(std::move(k), std::move(v));\
		}); \
    }); \
    }


JSON_ENCODE_MULTIMAP_FIELD(std::multimap);
JSON_DECODE_MULTIMAP_FIELD(std::multimap);

JSON_ENCODE_MULTIMAP_FIELD(std::unordered_multimap);
JSON_DECODE_MULTIMAP_FIELD(std::unordered_multimap);

#define JSON_ENCODE_ARRAY_FIELD(TYPE) \
    template<typename T>\
    inline shine::string json_encode_field(const TYPE<T> &val){\
if (val.empty()) return ""; \
    shine::string ret = "["; \
    bool empty = true; \
for (auto &iter : val)\
{\
    shine::string v = json_encode_field(iter); \
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


#define JSON_DECODE_ARRAY_FIELD(TYPE) \
    template<typename T>\
    inline void json_decode_field(TYPE<T> &val, shine::json_node_t *node){\
	val.clear();\
    node->foreach_array_childs([&val](const shine::size_type, const shine::json_node_t &value){\
    T v;\
    json_decode_field(v, (shine::json_node_t *)&value); \
    val.emplace_back(std::move(v)); \
    }); \
    }

JSON_ENCODE_ARRAY_FIELD(std::vector);
JSON_DECODE_ARRAY_FIELD(std::vector);

JSON_ENCODE_ARRAY_FIELD(std::deque);
JSON_DECODE_ARRAY_FIELD(std::deque);

JSON_ENCODE_ARRAY_FIELD(std::list);
JSON_DECODE_ARRAY_FIELD(std::list);

JSON_ENCODE_ARRAY_FIELD(std::forward_list);

template<typename T>
inline void json_decode_field(std::forward_list<T> &val, shine::json_node_t *node) {
	val.clear();
	node->foreach_array_childs([&val](const shine::size_type, const shine::json_node_t &value) {
		T v;
		json_decode_field(v, (shine::json_node_t *)&value);
		val.emplace_front(std::move(v));
	});
}

#define JSON_ENCODE_SET_FIELD(TYPE) JSON_ENCODE_ARRAY_FIELD(TYPE)


#define JSON_DECODE_SET_FIELD(TYPE) \
    template<typename T>\
    inline void json_decode_field(TYPE<T> &val, shine::json_node_t *node){\
	val.clear();\
    node->foreach_array_childs([&val](const shine::size_type, const shine::json_node_t &value){\
    T v;\
    json_decode_field(v, (shine::json_node_t *)&value); \
    val.emplace(std::move(v)); \
    }); \
    }

JSON_ENCODE_SET_FIELD(std::set);
JSON_DECODE_SET_FIELD(std::set);

JSON_ENCODE_SET_FIELD(std::unordered_set);
JSON_DECODE_SET_FIELD(std::unordered_set);

JSON_ENCODE_SET_FIELD(std::multiset);
JSON_DECODE_SET_FIELD(std::multiset);

JSON_ENCODE_SET_FIELD(std::unordered_multiset);
JSON_DECODE_SET_FIELD(std::unordered_multiset);

