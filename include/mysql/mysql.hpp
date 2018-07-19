 /**
 *****************************************************************************
 *
 *@note shineframe开发框架 https://github.com/shineframe/shineframe
 *
 *@file redis.hpp
 *
 *@brief mysql封装，实现代码参考引用了ACL库的逻辑实现
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
#include <map>
#include <unordered_map>
#include <vector>
#include <functional>
#include <mutex>
#include <thread>
#include <memory>
#include "../common/define.hpp"
#include "../net/socket.hpp"
#include "mysql_helper.hpp"

#if (defined SHINE_OS_WINDOWS)
#else
#endif


using namespace std;

namespace shine
{
    namespace db
    {
        typedef std::vector<string> row_t;
        typedef std::unordered_map<string, size_t, std::hash<std::string>, cmp_string> columns_t;
        typedef std::vector<row_t> rows_t;

        class mysql_result
        {
            friend class mysql;
        public:
            /** 
             *@brief 判断结果集中是否存在指定的列
             *@param column_name 
             *@return bool 
             *@warning 
             *@note 
            */
            bool has_column(const string & column_name){
                return _columns.find(column_name) != std::end(_columns);
            }

            /** 
             *@brief 获取结果集中的字段数量
             *@return size_t 
             *@warning 
             *@note 
            */
            size_t columns_size() const {
                return _columns.size();
            }

            /**
            *@brief 获取结果集中的总行数
            *@return size_t
            *@warning
            *@note
            */
            size_t rows_size() const {
                return _rows.size();
            }

            /** 
             *@brief 获取指定行数中指定名字段的值
             *@param row_num 
             *@param column_name 
             *@return const string * 
             *@warning 
             *@note 
            */
            const string * get(size_t row_num, const string & column_name){
                if (_rows.size() == 0 || row_num > _rows.size() - 1 || _columns.size() == 0)
                    return nullptr;

                auto iter = _columns.find(column_name);
                if (iter == std::end(_columns) || iter->second > _columns.size() - 1)
                    return nullptr;

                return &_rows[row_num][iter->second];
            }

            /** 
             *@brief 获取指定行数中指定列下标的字段的值
             *@param row_num 
             *@param column_num 
             *@return const string * 
             *@warning 
             *@note 
            */
            const string * get(size_t row_num, size_t column_num){
                if (_rows.size() == 0 || row_num > _rows.size() - 1 || _columns.size() == 0 || column_num > _columns.size() - 1)
                    return nullptr;

                return &_rows[row_num][column_num];
            }

            /** 
             *@brief 遍历所有的字段名
             *@param func 迭代处理函数
             *@return void 
             *@warning 
             *@note 
            */
            void foreach_colmuns(std::function<void(size_t index, const string &name)> func){
                if (func){
                    for (size_t i = 0; i < _head.size(); i++)
                        func(i, _head[i]);
                }                   
            }

            /**
            *@brief 遍历指定行的所有的字段
            *@param func 迭代处理函数
            *@return void
            *@warning
            *@note
            */
            void foreach_row(size_t row_num, std::function<void(size_t column_num, const string &column_name, const string &value)> func){
                if (func) {
                    if (_rows.size() == 0 || row_num > _rows.size() - 1)
                        return;

                    row_t &row = _rows[row_num];
                    for (size_t m = 0; m < row.size(); m++)
                        func(m, _head[m], row[m]);
                }
            }

            /**
            *@brief 遍历结果集中所有行所有列的数据
            *@param func 迭代处理函数
            *@return void
            *@warning
            *@note
            */
            void foreach_rows(std::function<void(size_t row, size_t column_num, const string &column_name, const string &value)> func){
                if (func) {
                    for (size_t i = 0; i < _rows.size(); i++)
                    {
                        row_t &row = _rows[i];
                        for (size_t m = 0; m < row.size(); m++)
                            func(i, m, _head[m], row[m]);
                    }
                }
            }

        private:
            void parse(MYSQL_RES *res){
                if (res == NULL)
                    return;

                size_t ncolumn = mysql_num_fields_func(res);
                _columns.clear();
                _head.resize(ncolumn);

                my_ulonglong nrow = mysql_num_rows_func(res);
                _rows.resize(nrow);

                MYSQL_FIELD *fields = mysql_fetch_fields_func(res);

                // 取出变量名
                for (size_t i = 0; i < ncolumn; i++)
                {
                    _columns.emplace(fields[i].name, i);
                    _head[i] = fields[i].name;
                }

                // 开始取出所有行数据结果，加入动态数组中
                size_type index = 0;
                while (true)
                {
                    MYSQL_ROW row = mysql_fetch_row_func(res);
                    if (row == NULL)
                        break;

                    _rows[index].resize(ncolumn);

                    for (size_t m = 0; m < ncolumn; m++)
                        _rows[index][m] = row[m];

                    index++;
                }

            }

        private:
            SHINE_GEN_MEMBER_BODY(columns_t, columns);
            SHINE_GEN_MEMBER_BODY(row_t, head);
            SHINE_GEN_MEMBER_BODY(rows_t, rows);

        };

        class mysql_connect_info{
            SHINE_GEN_MEMBER_GETSET(string, addr);
            SHINE_GEN_MEMBER_GETSET(string, database);
            SHINE_GEN_MEMBER_GETSET(string, user);
            SHINE_GEN_MEMBER_GETSET(string, password);
            SHINE_GEN_MEMBER_GETSET(string, charset, = "utf8");

            SHINE_GEN_MEMBER_GETSET(uLong, flags, = 0);
            SHINE_GEN_MEMBER_GETSET(uint32, conn_timeout , = 0);
            SHINE_GEN_MEMBER_GETSET(uint32, rw_timeout, = 0);
            SHINE_GEN_MEMBER_GETSET(bool, auto_commit, = true);
        };

        class mysql
        {
            friend class mysql_pool;
        private:
            
            mysql(const mysql_connect_info &connect_info)
            {
                get_connect_info() = connect_info;

                static once_flag t;
                std::call_once(t, []{
                    mysql_dll::load();
                });

            }

        public:

            ~mysql()
            {
                close();
            }

            uLong get_version() const
            {
                return mysql_get_client_version_func();
            }

            const int8* get_client_info() const
            {
                return mysql_get_client_info_func();
            }

            int32 get_errno() const
            {
                if (get_conn())
                    return mysql_errno_func(get_conn());
                else
                    return -1;
            }

            const int8* get_error() const
            {
                if (get_conn())
                    return mysql_error_func(get_conn());
                else
                    return "not init";
            }

            bool open()
            {
                if (get_conn() != NULL)
                    return true;

                if (get_conn() == NULL)
                {
                    get_conn() = mysql_init_func(NULL);
                    if (get_conn() == NULL)
                        return false;
                }

                net::address_info_t addr;
                bool unix_socket = !net::socket::parse_addr(get_connect_info().get_addr(), addr);

                if (get_connect_info().get_conn_timeout() > 0)
                    mysql_options_func(get_conn(), MYSQL_OPT_CONNECT_TIMEOUT, (const void*)&get_connect_info().get_conn_timeout());

                if (get_connect_info().get_rw_timeout() > 0)
                {
                    mysql_options_func(get_conn(), MYSQL_OPT_READ_TIMEOUT, (const void*)&get_connect_info().get_rw_timeout());
                    mysql_options_func(get_conn(), MYSQL_OPT_WRITE_TIMEOUT, (const void*)&get_connect_info().get_rw_timeout());
                }

                my_bool reconnect = 1;

                mysql_options_func(get_conn(), MYSQL_OPT_RECONNECT, (const void*)&reconnect);

                if (mysql_real_connect_func(get_conn(), 
                    unix_socket ? NULL : addr.get_ip().c_str(), 
                    get_connect_info().get_user().c_str(),
                    get_connect_info().get_password().c_str(),
                    get_connect_info().get_database().c_str(),
                    unix_socket ? 0 : addr.get_port(),
                    unix_socket ? get_connect_info().get_addr().c_str() : NULL,
                    get_connect_info().get_flags()) == NULL)
                {
                    mysql_close_func(get_conn());
                    get_conn() = NULL;
                    return false;
                }

                if (!get_connect_info().get_charset().empty())
                {
                    if (mysql_set_character_set_func(get_conn(), get_connect_info().get_charset().c_str()))
                        printf("set mysql to %s error %s", get_connect_info().get_charset().c_str(), mysql_error_func(get_conn()));
                }

#if MYSQL_VERSION_ID >= 50000
                if (mysql_autocommit_func(get_conn(), get_connect_info().get_auto_commit() ? 1 : 0) != 0)
                {
                    mysql_close_func(get_conn());
                    get_conn() = NULL;
                    return (false);
                }
#else
                get_connect_info().get_auto_commit() = false;
#endif

                return true;
            }

            bool is_opened() const
            {
                return get_conn() ? true : false;
            }

            bool close()
            {
                if (get_conn() != NULL)
                {
                    mysql_close_func(get_conn());
                    get_conn() = NULL;
                }
                return true;
            }

            bool request(const string& sql)
            {
                if (get_conn() == NULL)
                    return false;

                if (mysql_query_func(get_conn(), sql.c_str()) == 0)
                    return true;

                int errnum = mysql_errno_func(get_conn());
                if (errnum != CR_SERVER_LOST && errnum != CR_SERVER_GONE_ERROR)
                {
                    printf("%s", mysql_error_func(get_conn()));
                    return false;
                }

                close();

                if (open() == false)
                    return false;

                if (mysql_query_func(get_conn(), sql.c_str()) == 0)
                    return true;

                return false;
            }

            bool exists(const string &table)
            {
                if (get_conn() == NULL)
                    return false;

                string sql;
                sql.format("show tables like '%s'", table.c_str());

                if (request(sql.c_str()) == false)
                    return false;

                MYSQL_RES *res = mysql_store_result_func(get_conn());
                if (res == NULL)
                {
                    if (mysql_errno_func(get_conn()) != 0)
                        close();
                    return false;
                }

                bool ret = mysql_num_rows_func(res) > 0;
                mysql_free_result_func(res);
                return ret;
            }

            bool select(const string &sql, mysql_result *result = NULL)
            {
                if (request(sql) == false)
                    return false;

                MYSQL_RES *res = mysql_store_result_func(get_conn());
                if (res == NULL)
                {
                    if (mysql_errno_func(get_conn()) != 0)
                        close();
                    return false;
                }

                if (result != NULL)
                    result->parse(res);

                mysql_free_result_func(res);

                return true;
            }

            bool execute(const string &sql)
            {
                if (request(sql) == false)
                    return false;

                int ret = (int)mysql_affected_rows_func(get_conn());
                if (ret == -1)
                    return false;

                return true;
            }

            int32 affect_count() const
            {
                if (!is_opened())
                    return -1;

                return (int)mysql_affected_rows_func(get_conn());
            }

            bool begin_transaction()
            {
                static const string sql = "start transaction";
                return execute(sql);
            }

            bool commit()
            {
                static const string sql = "commit";
                return execute(sql);
            }

private:
    typedef MYSQL* MYSQL_t;
    SHINE_GEN_MEMBER_GETSET(mysql_connect_info, connect_info);
    SHINE_GEN_MEMBER_GETSET(MYSQL_t, conn, = NULL);
        };

        /**
        *
         @brief mysql对象池，当需要连接不同数据库地址或者数据库名时需要创建多个mysql对象池，以区分连接
         *
         * todo: 
         *
         */
        
        class mysql_pool{
        public:
            typedef std::shared_ptr<mysql> mysql_ptr_t;
            typedef std::unordered_map<std::thread::id, mysql_ptr_t > pool_t;

            mysql_pool(const mysql_connect_info &connect_info){
                _connect_info = connect_info;
            }

            /** 
             *@brief 获取一个mysql连接对象
             *@return std::shared_ptr<mysql> 
             *@warning 
             *@note 
            */
            std::shared_ptr<mysql> get(){

                shared_ptr<mysql> ret;
                std::unique_lock<std::recursive_mutex> lock(_mutex);
                auto iter = _pool.find(std::this_thread::get_id());
                if (iter == _pool.end())
                {
                    ret.reset(new mysql(_connect_info));
                    ret->open();
                    _pool[std::this_thread::get_id()] = ret;
                    return std::move(ret);
                }

                ret = iter->second;
                return std::move(ret);
            }

            void clear(){
                _pool.clear();
            }

        private:
            std::recursive_mutex _mutex;
            mysql_connect_info _connect_info;
            pool_t _pool;
        };

    }
}
