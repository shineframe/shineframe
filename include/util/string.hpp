 /**
 *****************************************************************************
 *
 *@file string.hpp
 *
 *@brief ×Ö·û´®·â×°
 *
 *@todo 
 * 
 *@note shineframe¿ª·¢¿ò¼Ü https://github.com/shineframe/shineframe
 *
 *@author sunjian 39215174@qq.com
 *
 *@version 1.0
 *
 *@date 2018/6/15 
 *****************************************************************************
 */
#pragma once
#include <string.h>
#include <string>
#include <stdarg.h>
#include <vector>
#include <algorithm>
#include <locale>
#include "../common/define.hpp"
#include "tool.hpp"
#include "md5.hpp"

const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789+/";

namespace shine
{
    class string : public std::string
    {
    public:
        string() : std::string(){}
        string(const int8* str) : std::string(str) {}
        string(const std::string &other) : std::string(other){}
        string(std::string &&other) : std::string(other){}
        string(const string &other) : std::string(other){}
        string(string &&other) : std::string(other){}

        string (int16 v){
            std::string::assign(std::move(number_to_string("%d", v)));
        }

        string (uint16 v){
            std::string::assign(std::move(number_to_string("%u", v)));
        }

        string(int32 v){
            std::string::assign(std::move(number_to_string("%d", v)));
        }

        string(uint32 v){
            std::string::assign(std::move(number_to_string("%u", v)));
        }

        string(Long v){
            std::string::assign(std::move(number_to_string("%ld", v)));
        }

        string(uLong v){
            std::string::assign(std::move(number_to_string("%lu", v)));
        }

        string (int64 v){
            std::string::assign(std::move(number_to_string("%lld", v)));
        }

        string (uint64 v){
            std::string::assign(std::move(number_to_string("%lu", v)));
        }

//         string(size_type v){
//             std::string::assign(std::to_string(v));
//         }

        string (Float v){
            std::string::assign(std::move(number_to_string("%f", v)));
        }

        string(Double v){
            std::string::assign(std::move(number_to_string("%f", v)));
        }

        string(LDouble v){
            std::string::assign(std::move(number_to_string("%Lf", v)));
        }

        template<class T>
        std::string number_to_string(const int8 *fmt, const T &num){
            int8 buf[64];
            size_t len = SHINE_SNPRINTF(buf, sizeof(buf), fmt, num);
            if (strstr(buf, ".") != NULL)
            {
                while (len > 1 && buf[--len] == '0')
                    buf[len] = '\0';
            }

            return std::move(std::string(buf));
        }

        int16 to_int16(){
            return (int16)std::stoi(*this);
        }

        uint16 to_uint16(){
            return (uint16)std::stoul(*this);
        }

        int32 to_int32(){
            return std::stoi(*this);
        }

        uint32 to_uint32(){
            return (uint32)std::stoul(*this);
        }

        Long to_long(){
            return std::stol(*this);
        }

        uLong to_ulong(){
            return std::stoul(*this);
        }

        int64 to_int64(){
            return std::stoll(*this);
        }

        uint64 to_uint64(){
            return (uint32)std::stoull(*this);
        }

        Float to_float(){
            return std::stof(*this);
        }

        Double to_double(){
            return std::stod(*this);
        }

        LDouble to_long_double(){
            return std::stold(*this);
        }

        string &operator=(const string &v){
            std::string::assign(v.data(), v.size());
            return *this;
        }

        string &operator=(int8 v){
            std::string::assign(1, v);
            return *this;
        }

        string &operator=(uint8 v){
            std::string::assign(1, v);
            return *this;
        }

        string &operator=(int16 v){
            std::string::assign(number_to_string("%d", v));
            return *this;
        }

        string &operator=(uint16 v){
            std::string::assign(number_to_string("%u", v));
            return *this;
        }

        string &operator=(int32 v){

            std::string::assign(number_to_string("%d", v));
            return *this;
        }

        string &operator=(uint32 v){
            std::string::assign(number_to_string("%u", v));
            return *this;
        }

        string &operator=(Long v){

            std::string::assign(number_to_string("%ld", v));
            return *this;
        }

        string &operator=(uLong v){
            std::string::assign(number_to_string("%lu", v));
            return *this;
        }

        string &operator=(int64 v){
            std::string::assign(number_to_string("%lld", v));
            return *this;
        }

        string &operator=(uint64 v){
            std::string::assign(number_to_string("%lu", v));
            return *this;
        }

        string &operator=(Float v){
            std::string::assign(number_to_string("%f", v));
            return *this;
        }

        string &operator=(Double v){
            std::string::assign(number_to_string("%f", v));
            return *this;
        }

        string &operator=(LDouble v){
            std::string::assign(number_to_string("%Lf", v));
            return *this;
        }

        string &operator<<(const int8 *v){
            std::string::append(v);
            return *this;
        }

        string &operator<<(int8 v){
            std::string::append(1, v);
            return *this;
        }

        string &operator<<(uint8 v){
            std::string::append(1, v);
            return *this;
        }

        string &operator<<(int16 v){
            std::string::append(number_to_string("%d", v));
            return *this;
        }

        string &operator<<(uint16 v){
            std::string::append(number_to_string("%u", v));
            return *this;
        }

        string &operator<<(int32 v){
            std::string::append(number_to_string("%d", v));
            return *this;
        }

        string &operator<<(uint32 v){
            std::string::append(number_to_string("%u", v));
            return *this;
        }

        string &operator<<(Long v){
            std::string::append(number_to_string("%ld", v));
            return *this;
        }

        string &operator<<(uLong v){
            std::string::append(number_to_string("%lu", v));
            return *this;
        }

        string &operator<<(int64 v){
            std::string::append(number_to_string("%lld", v));
            return *this;
        }

        string &operator<<(uint64 v){
            std::string::append(number_to_string("%lu", v));
            return *this;
        }

        string &operator<<(Float v){
            std::string::append(number_to_string("%f", v));
            return *this;
        }

        string &operator<<(Double v){
            std::string::append(number_to_string("%f", v));
            return *this;
        }

        string &operator<<(LDouble v){
            std::string::append(number_to_string("%Lf", v));
            return *this;
        }

        string &operator+=(int8 v){
            std::string::append(1, v);
            return *this;
        }

        string &operator+=(uint8 v){
            std::string::append(1, v);
            return *this;
        }

        string &operator+=(int16 v){
            std::string::append(number_to_string("%d", v));
            return *this;
        }

        string &operator+=(uint16 v){
            std::string::append(number_to_string("%u", v));
            return *this;
        }

        string &operator+=(int32 v){
            std::string::append(number_to_string("%d", v));
            return *this;
        }

        string &operator+=(uint32 v){
            std::string::append(number_to_string("%u", v));
            return *this;
        }

        string &operator+=(Long v){
            std::string::append(number_to_string("%ld", v));
            return *this;
        }

        string &operator+=(uLong v){
            std::string::append(number_to_string("%lu", v));
            return *this;
        }

        string &operator+=(int64 v){
            std::string::append(number_to_string("%lld", v));
            return *this;
        }

        string &operator+=(uint64 v){
            std::string::append(number_to_string("%lu", v));
            return *this;
        }

        string &operator+=(Float v){
            std::string::append(number_to_string("%f", v));
            return *this;
        }

        string &operator+=(Double v){
            std::string::append(number_to_string("%f", v));
            return *this;
        }

        string &operator+=(LDouble v){
            std::string::append(number_to_string("%Lf", v));
            return *this;
        }

        string &operator+=(const int8 *v){
            std::string::append(v);
            return *this;
        }

        string &operator+=(const string &v){
            std::string::append(v.data(), v.size());
            return *this;
        }

        static std::vector<string> split(const string &str , const std::string &des){
            std::vector<string> ret;
            std::string::size_type des_len = des.size();
            std::string::size_type begin = 0;
            std::string::size_type end = str.find(des, begin);
            
            while (end != std::string::npos)
            {
                ret.push_back(str.substr(begin, end - begin));
                begin = end + des_len;
                end = str.find(des, begin);
            }

            ret.push_back(str.substr(begin, end));

            return ret;
        }

        std::vector<string> split(const std::string &des){
            return std::move(split(*this, des));
        }

        static size_type assert_size(const int8 *fmt, va_list args)
        {
#if (defined SHINE_OS_WINDOWS)
#pragma warning(disable:4996)
#endif
            return vsnprintf(0, 0, fmt, args);
        }

        static void format(string &str, const int8 *fmt, va_list args)
        {
            SHINE_VSNPRINTF((char*)str.c_str(), str.size(), fmt, args);
        }

        static string format_create(const int8 *fmt, ...){
            string ret;
            va_list args;
            va_start(args, fmt);
            size_type size = assert_size(fmt, args);
            va_end(args);

            va_start(args, fmt);
            ret.resize(size + 1);
            format(ret, fmt, args);
            va_end(args);

            ret.resize(size);
            return std::move(ret);
        }

        string& format(const int8 *fmt, ...){
            va_list args;
            va_start(args, fmt);
            size_type size = assert_size(fmt, args);
            va_end(args);

            va_start(args, fmt);
            resize(size + 1);
            format(*this, fmt, args);
            va_end(args);

            resize(size);

            return *this;
        }


        string& format_append(const int8 *fmt, ...){
            string tmp;
            va_list args;
            va_start(args, fmt);
            size_type size = assert_size(fmt, args);
            va_end(args);

            va_start(args, fmt);
            tmp.resize(size + 1);
            format(tmp, fmt, args);
            va_end(args);
            tmp.resize(size);

            append(tmp);
            return *this;

        }

        static void replace_all(string &str, const std::string &src, const std::string &des) {
            string::size_type pos = 0;
            string::size_type src_len = src.size();
            string::size_type des_len = des.size();
            pos = str.find(src, pos);
            while ((pos != string::npos))
            {
                str.replace(pos, src_len, des);
                pos = str.find(src, (pos + des_len));
            }
        }

        string& replace_all(const std::string &src, const std::string &des) {
            replace_all(*this, src, des);
            return *this;
        }

        string& to_upper(){
            std::transform(begin(), end(), begin(), ::toupper);
            return *this;
        }

        string& to_lower(){
            std::transform(begin(), end(), begin(), ::tolower);
            return *this;
        }

        string& trim_left() {
            if (!empty())
                erase(0, find_first_not_of(" \n\r\t"));

            return *this;
        }

        string& trim_right() {
            if (!empty())
                erase(find_last_not_of(" \n\r\t") + 1);

            return *this;
        }

        string& trim() {
            return trim_left().trim_right();
        }

        bool contains(const char* text){
            return find(text) != std::string::npos;
        }

        string md5_16() const {
            return md5_16(*this);
        }

        static string md5_16(const string &str){
            return md5(str).to_string_16();
        }

        string md5_32() const {
            return md5_32(*this);
        }

        static string md5_32(const string &str){
            return md5(str).to_string_32();
        }

        string encode_base64(){
            return encode_base64(*this);
        }

        static string encode_base64(const string &data) {
            const uint8* bytes_to_encode = (const uint8*)data.data();
            size_t in_len = data.size();

            string ret;
            int32 i = 0;
            int32 j = 0;
            uint8 char_array_3[3];
            uint8 char_array_4[4];

            while (in_len--) {
                char_array_3[i++] = *(bytes_to_encode++);
                if (i == 3) {
                    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                    char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
                    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
                    char_array_4[3] = char_array_3[2] & 0x3f;

                    for (i = 0; (i < 4); i++)
                        ret += base64_chars[char_array_4[i]];
                    i = 0;
                }
            }

            if (i)
            {
                for (j = i; j < 3; j++)
                    char_array_3[j] = '\0';

                char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
                char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
                char_array_4[3] = char_array_3[2] & 0x3f;

                for (j = 0; (j < i + 1); j++)
                    ret += base64_chars[char_array_4[j]];

                while ((i++ < 3))
                    ret += '=';

            }

            return std::move(ret);

        }


        string decode_base64(){
            return decode_base64(*this);
        }

        static string decode_base64(const string &data) {
            size_type in_len = data.size();
            int32 i = 0;
            int32 j = 0;
            int32 in_ = 0;
            uint8 char_array_4[4], char_array_3[3];
            string ret;

            while (in_len-- && (data[in_] != '=') && is_base64(data[in_])) {
                char_array_4[i++] = data[in_]; in_++;
                if (i == 4) {
                    for (i = 0; i < 4; i++)
                        char_array_4[i] = static_cast<uint8>(base64_chars.find(char_array_4[i]));

                    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
                    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
                    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

                    for (i = 0; (i < 3); i++)
                        ret += char_array_3[i];
                    i = 0;
                }
            }

            if (i) {
                for (j = i; j < 4; j++)
                    char_array_4[j] = 0;

                for (j = 0; j < 4; j++)
                    char_array_4[j] = static_cast<uint8>(base64_chars.find(char_array_4[j]));

                char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
                char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
                char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

                for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
            }

            return std::move(ret);
        }

    private:
        static inline bool is_base64(uint8 c) {
            return (isalnum(c) || (c == '+') || (c == '/'));
        }
    };

    struct cmp_string
    {
        bool operator()(const string &rc1, const string &rc2) const
        {
            return rc1 == rc2;
        }
    };

}

/*

namespace std{

// TEMPLATE STRUCT SPECIALIZATION hash
template<>
struct hash<shine::string>
    : public unary_function<std::string, size_t>
{	// hash functor for basic_string
    typedef shine::string _Kty;

    size_t operator()(const _Kty& _Keyval) const
    {	// hash _Keyval to size_t value by pseudorandomizing transform
        return (_Hash_seq((const unsigned char *)_Keyval.c_str(),
            _Keyval.size() * sizeof (char)));
    }
};

}*/