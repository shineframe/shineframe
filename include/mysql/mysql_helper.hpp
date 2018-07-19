 /**
 *****************************************************************************
 *
 *@note shineframe开发框架 https://github.com/shineframe/shineframe
 *
 *@file redis.hpp
 *
 *@brief mysql铺助
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
#include "../util/dll.hpp"
#include "mysql/mysql.h"
#include "mysql/errmsg.h"


#if (defined SHINE_OS_WINDOWS)
#else
#endif

typedef MYSQL_PARAMETERS *(STDCALL * mysql_get_parameters_t)(void);
typedef my_bool(STDCALL * mysql_thread_init_t)(void);
typedef void (STDCALL * mysql_thread_end_t)(void);
typedef my_ulonglong(STDCALL * mysql_num_rows_t)(MYSQL_RES *res);
typedef unsigned int (STDCALL * mysql_num_fields_t)(MYSQL_RES *res);
typedef my_bool(STDCALL * mysql_eof_t)(MYSQL_RES *res);
typedef MYSQL_FIELD *(STDCALL * mysql_fetch_field_direct_t)(MYSQL_RES *res, unsigned int fieldnr);
typedef MYSQL_FIELD * (STDCALL * mysql_fetch_fields_t)(MYSQL_RES *res);
typedef MYSQL_ROW_OFFSET(STDCALL * mysql_row_tell_t)(MYSQL_RES *res);
typedef MYSQL_FIELD_OFFSET(STDCALL * mysql_field_tell_t)(MYSQL_RES *res);
typedef unsigned int (STDCALL * mysql_field_count_t)(MYSQL *mysql);
typedef my_ulonglong(STDCALL * mysql_affected_rows_t)(MYSQL *mysql);
typedef my_ulonglong(STDCALL * mysql_insert_id_t)(MYSQL *mysql);
typedef unsigned int (STDCALL * mysql_errno_t)(MYSQL *mysql);
typedef const char * (STDCALL * mysql_error_t)(MYSQL *mysql);
typedef const char *(STDCALL * mysql_sqlstate_t)(MYSQL *mysql);
typedef unsigned int (STDCALL * mysql_warning_count_t)(MYSQL *mysql);
typedef const char * (STDCALL * mysql_info_t)(MYSQL *mysql);
typedef unsigned long (STDCALL * mysql_thread_id_t)(MYSQL *mysql);
typedef const char * (STDCALL * mysql_character_set_name_t)(MYSQL *mysql);
typedef int (STDCALL * mysql_set_character_set_t)(MYSQL *mysql, const char *csname);
typedef MYSQL * (STDCALL * mysql_init_t)(MYSQL *mysql);
typedef my_bool(STDCALL * mysql_ssl_set_t)(MYSQL *mysql, const char *key, const char *cert, const char *ca, const char *capath, const char *cipher);
typedef const char *  (STDCALL * mysql_get_ssl_cipher_t)(MYSQL *mysql);
typedef my_bool(STDCALL * mysql_change_user_t)(MYSQL *mysql, const char *user, const char *passwd, const char *db);
typedef MYSQL * (STDCALL * mysql_real_connect_t)(MYSQL *mysql, const char *host, const char *user, const char *passwd, const char *db, unsigned int port, const char *unix_socket, unsigned long clientflag);
typedef int (STDCALL * mysql_select_db_t)(MYSQL *mysql, const char *db);
typedef int (STDCALL * mysql_query_t)(MYSQL *mysql, const char *q);
typedef int (STDCALL * mysql_send_query_t)(MYSQL *mysql, const char *q, unsigned long length);
typedef int (STDCALL * mysql_real_query_t)(MYSQL *mysql, const char *q, unsigned long length);
typedef MYSQL_RES * (STDCALL * mysql_store_result_t)(MYSQL *mysql);
typedef MYSQL_RES * (STDCALL * mysql_use_result_t)(MYSQL *mysql);
typedef void (STDCALL * mysql_get_character_set_info_t)(MYSQL *mysql, MY_CHARSET_INFO *charset);
typedef int (STDCALL * mysql_shutdown_t)(MYSQL *mysql, enum mysql_enum_shutdown_level shutdown_level);
typedef int (STDCALL * mysql_dump_debug_info_t)(MYSQL *mysql);
typedef int (STDCALL * mysql_refresh_t)(MYSQL *mysql, unsigned int refresh_options);
typedef int (STDCALL * mysql_kill_t)(MYSQL *mysql, unsigned long pid);
typedef int (STDCALL * mysql_set_server_option_t)(MYSQL *mysql, enum enum_mysql_set_option option);
typedef int (STDCALL * mysql_ping_t)(MYSQL *mysql);
typedef const char * (STDCALL * mysql_stat_t)(MYSQL *mysql);
typedef const char * (STDCALL * mysql_get_server_info_t)(MYSQL *mysql);
typedef const char * (STDCALL * mysql_get_client_info_t)(void);
typedef unsigned long (STDCALL * mysql_get_client_version_t)(void);
typedef const char * (STDCALL * mysql_get_host_info_t)(MYSQL *mysql);
typedef unsigned long (STDCALL * mysql_get_server_version_t)(MYSQL *mysql);
typedef unsigned int (STDCALL * mysql_get_proto_info_t)(MYSQL *mysql);
typedef MYSQL_RES * (STDCALL * mysql_list_dbs_t)(MYSQL *mysql, const char *wild);
typedef MYSQL_RES * (STDCALL * mysql_list_tables_t)(MYSQL *mysql, const char *wild);
typedef MYSQL_RES * (STDCALL * mysql_list_processes_t)(MYSQL *mysql);
typedef int (STDCALL * mysql_options_t)(MYSQL *mysql, enum mysql_option option, const void *arg);
typedef void (STDCALL * mysql_free_result_t)(MYSQL_RES *result);
typedef void (STDCALL * mysql_data_seek_t)(MYSQL_RES *result, my_ulonglong offset);
typedef MYSQL_ROW_OFFSET(STDCALL * mysql_row_seek_t)(MYSQL_RES *result, MYSQL_ROW_OFFSET offset);
typedef MYSQL_FIELD_OFFSET(STDCALL * mysql_field_seek_t)(MYSQL_RES *result, MYSQL_FIELD_OFFSET offset);
typedef MYSQL_ROW(STDCALL * mysql_fetch_row_t)(MYSQL_RES *result);
typedef unsigned long * (STDCALL * mysql_fetch_lengths_t)(MYSQL_RES *result);
typedef MYSQL_FIELD * (STDCALL * mysql_fetch_field_t)(MYSQL_RES *result);
typedef MYSQL_RES * (STDCALL * mysql_list_fields_t)(MYSQL *mysql, const char *table, const char *wild);
typedef unsigned long (STDCALL * mysql_escape_string_t)(char *to, const char *from, unsigned long from_length);
typedef unsigned long (STDCALL * mysql_hex_string_t)(char *to, const char *from, unsigned long from_length);
typedef unsigned long (STDCALL * mysql_real_escape_string_t)(MYSQL *mysql, char *to, const char *from, unsigned long length);
typedef void (STDCALL * mysql_debug_t)(const char *debug);
typedef void (STDCALL * myodbc_remove_escape_t)(MYSQL *mysql, char *name);
typedef unsigned int (STDCALL * mysql_thread_safe_t)(void);
typedef my_bool(STDCALL * mysql_embedded_t)(void);
typedef my_bool(STDCALL * mysql_read_query_result_t)(MYSQL *mysql);
typedef MYSQL_STMT * (STDCALL * mysql_stmt_init_t)(MYSQL *mysql);
typedef int (STDCALL * mysql_stmt_prepare_t)(MYSQL_STMT *stmt, const char *query, unsigned long length);
typedef int (STDCALL * mysql_stmt_execute_t)(MYSQL_STMT *stmt);
typedef int (STDCALL * mysql_stmt_fetch_t)(MYSQL_STMT *stmt);
typedef int (STDCALL * mysql_stmt_fetch_column_t)(MYSQL_STMT *stmt, MYSQL_BIND *bind_arg, unsigned int column, unsigned long offset);
typedef int (STDCALL * mysql_stmt_store_result_t)(MYSQL_STMT *stmt);
typedef unsigned long (STDCALL * mysql_stmt_param_count_t)(MYSQL_STMT * stmt);
typedef my_bool(STDCALL * mysql_stmt_attr_set_t)(MYSQL_STMT *stmt, enum enum_stmt_attr_type attr_type, const void *attr);
typedef my_bool(STDCALL * mysql_stmt_attr_get_t)(MYSQL_STMT *stmt, enum enum_stmt_attr_type attr_type, void *attr);
typedef my_bool(STDCALL * mysql_stmt_bind_param_t)(MYSQL_STMT * stmt, MYSQL_BIND * bnd);
typedef my_bool(STDCALL * mysql_stmt_bind_result_t)(MYSQL_STMT * stmt, MYSQL_BIND * bnd);
typedef my_bool(STDCALL * mysql_stmt_close_t)(MYSQL_STMT * stmt);
typedef my_bool(STDCALL * mysql_stmt_reset_t)(MYSQL_STMT * stmt);
typedef my_bool(STDCALL * mysql_stmt_free_result_t)(MYSQL_STMT *stmt);
typedef my_bool(STDCALL * mysql_stmt_send_long_data_t)(MYSQL_STMT *stmt, unsigned int param_number, const char *data, unsigned long length);
typedef MYSQL_RES *(STDCALL * mysql_stmt_result_metadata_t)(MYSQL_STMT *stmt);
typedef MYSQL_RES *(STDCALL * mysql_stmt_param_metadata_t)(MYSQL_STMT *stmt);
typedef unsigned int (STDCALL * mysql_stmt_errno_t)(MYSQL_STMT * stmt);
typedef const char *(STDCALL * mysql_stmt_error_t)(MYSQL_STMT * stmt);
typedef const char *(STDCALL * mysql_stmt_sqlstate_t)(MYSQL_STMT * stmt);
typedef MYSQL_ROW_OFFSET(STDCALL * mysql_stmt_row_seek_t)(MYSQL_STMT *stmt, MYSQL_ROW_OFFSET offset);
typedef MYSQL_ROW_OFFSET(STDCALL * mysql_stmt_row_tell_t)(MYSQL_STMT *stmt);
typedef void (STDCALL * mysql_stmt_data_seek_t)(MYSQL_STMT *stmt, my_ulonglong offset);
typedef my_ulonglong(STDCALL * mysql_stmt_num_rows_t)(MYSQL_STMT *stmt);
typedef my_ulonglong(STDCALL * mysql_stmt_affected_rows_t)(MYSQL_STMT *stmt);
typedef my_ulonglong(STDCALL * mysql_stmt_insert_id_t)(MYSQL_STMT *stmt);
typedef unsigned int (STDCALL * mysql_stmt_field_count_t)(MYSQL_STMT *stmt);
typedef my_bool(STDCALL * mysql_commit_t)(MYSQL * mysql);
typedef my_bool(STDCALL * mysql_rollback_t)(MYSQL * mysql);
typedef my_bool(STDCALL * mysql_autocommit_t)(MYSQL * mysql, my_bool auto_mode);
typedef my_bool(STDCALL * mysql_more_results_t)(MYSQL *mysql);
typedef int (STDCALL * mysql_next_result_t)(MYSQL *mysql);
typedef int (STDCALL * mysql_stmt_next_result_t)(MYSQL_STMT *stmt);
typedef void (STDCALL * mysql_close_t)(MYSQL *sock);

#define SHINE_MYSQL_FUNC_DEF(name) static name##_t name##_func = NULL;
#define SHINE_MYSQL_FUNC_LOAD(name) { name##_func = (name##_t) dll::sym(mysql_dll_handle, #name);\
if (name##_func == NULL)\
{\
    printf("load %s error: %d", #name, dll::get_error());\
    return false;\
}\
}

SHINE_MYSQL_FUNC_DEF(mysql_get_parameters);
SHINE_MYSQL_FUNC_DEF(mysql_thread_init);
SHINE_MYSQL_FUNC_DEF(mysql_thread_end);
SHINE_MYSQL_FUNC_DEF(mysql_num_rows);
SHINE_MYSQL_FUNC_DEF(mysql_num_fields);
SHINE_MYSQL_FUNC_DEF(mysql_eof);
SHINE_MYSQL_FUNC_DEF(mysql_fetch_field_direct);
SHINE_MYSQL_FUNC_DEF(mysql_fetch_fields);
SHINE_MYSQL_FUNC_DEF(mysql_row_tell);
SHINE_MYSQL_FUNC_DEF(mysql_field_tell);
SHINE_MYSQL_FUNC_DEF(mysql_field_count);
SHINE_MYSQL_FUNC_DEF(mysql_affected_rows);
SHINE_MYSQL_FUNC_DEF(mysql_insert_id);
SHINE_MYSQL_FUNC_DEF(mysql_errno);
SHINE_MYSQL_FUNC_DEF(mysql_error);
SHINE_MYSQL_FUNC_DEF(mysql_sqlstate);
SHINE_MYSQL_FUNC_DEF(mysql_warning_count);
SHINE_MYSQL_FUNC_DEF(mysql_info);
SHINE_MYSQL_FUNC_DEF(mysql_thread_id);
SHINE_MYSQL_FUNC_DEF(mysql_character_set_name);
SHINE_MYSQL_FUNC_DEF(mysql_set_character_set);
SHINE_MYSQL_FUNC_DEF(mysql_init);
SHINE_MYSQL_FUNC_DEF(mysql_ssl_set);
SHINE_MYSQL_FUNC_DEF(mysql_get_ssl_cipher);
SHINE_MYSQL_FUNC_DEF(mysql_change_user);
SHINE_MYSQL_FUNC_DEF(mysql_real_connect);
SHINE_MYSQL_FUNC_DEF(mysql_select_db);
SHINE_MYSQL_FUNC_DEF(mysql_query);
SHINE_MYSQL_FUNC_DEF(mysql_send_query);
SHINE_MYSQL_FUNC_DEF(mysql_real_query);
SHINE_MYSQL_FUNC_DEF(mysql_store_result);
SHINE_MYSQL_FUNC_DEF(mysql_use_result);
SHINE_MYSQL_FUNC_DEF(mysql_get_character_set_info);
SHINE_MYSQL_FUNC_DEF(mysql_shutdown);
SHINE_MYSQL_FUNC_DEF(mysql_dump_debug_info);
SHINE_MYSQL_FUNC_DEF(mysql_refresh);
SHINE_MYSQL_FUNC_DEF(mysql_kill);
SHINE_MYSQL_FUNC_DEF(mysql_set_server_option);
SHINE_MYSQL_FUNC_DEF(mysql_ping);
SHINE_MYSQL_FUNC_DEF(mysql_stat);
SHINE_MYSQL_FUNC_DEF(mysql_get_server_info);
SHINE_MYSQL_FUNC_DEF(mysql_get_client_info);
SHINE_MYSQL_FUNC_DEF(mysql_get_client_version);
SHINE_MYSQL_FUNC_DEF(mysql_get_host_info);
SHINE_MYSQL_FUNC_DEF(mysql_get_server_version);
SHINE_MYSQL_FUNC_DEF(mysql_get_proto_info);
SHINE_MYSQL_FUNC_DEF(mysql_list_dbs);
SHINE_MYSQL_FUNC_DEF(mysql_list_tables);
SHINE_MYSQL_FUNC_DEF(mysql_list_processes);
SHINE_MYSQL_FUNC_DEF(mysql_options);
SHINE_MYSQL_FUNC_DEF(mysql_free_result);
SHINE_MYSQL_FUNC_DEF(mysql_data_seek);
SHINE_MYSQL_FUNC_DEF(mysql_row_seek);
SHINE_MYSQL_FUNC_DEF(mysql_field_seek);
SHINE_MYSQL_FUNC_DEF(mysql_fetch_row);
SHINE_MYSQL_FUNC_DEF(mysql_fetch_lengths);
SHINE_MYSQL_FUNC_DEF(mysql_fetch_field);
SHINE_MYSQL_FUNC_DEF(mysql_list_fields);
SHINE_MYSQL_FUNC_DEF(mysql_escape_string);
SHINE_MYSQL_FUNC_DEF(mysql_hex_string);
SHINE_MYSQL_FUNC_DEF(mysql_real_escape_string);
SHINE_MYSQL_FUNC_DEF(mysql_debug);
SHINE_MYSQL_FUNC_DEF(myodbc_remove_escape);
SHINE_MYSQL_FUNC_DEF(mysql_thread_safe);
SHINE_MYSQL_FUNC_DEF(mysql_embedded);
SHINE_MYSQL_FUNC_DEF(mysql_read_query_result);
SHINE_MYSQL_FUNC_DEF(mysql_stmt_init);
SHINE_MYSQL_FUNC_DEF(mysql_stmt_prepare);
SHINE_MYSQL_FUNC_DEF(mysql_stmt_execute);
SHINE_MYSQL_FUNC_DEF(mysql_stmt_fetch);
SHINE_MYSQL_FUNC_DEF(mysql_stmt_fetch_column);
SHINE_MYSQL_FUNC_DEF(mysql_stmt_store_result);
SHINE_MYSQL_FUNC_DEF(mysql_stmt_param_count);
SHINE_MYSQL_FUNC_DEF(mysql_stmt_attr_set);
SHINE_MYSQL_FUNC_DEF(mysql_stmt_attr_get);
SHINE_MYSQL_FUNC_DEF(mysql_stmt_bind_param);
SHINE_MYSQL_FUNC_DEF(mysql_stmt_bind_result);
SHINE_MYSQL_FUNC_DEF(mysql_stmt_close);
SHINE_MYSQL_FUNC_DEF(mysql_stmt_reset);
SHINE_MYSQL_FUNC_DEF(mysql_stmt_free_result);
SHINE_MYSQL_FUNC_DEF(mysql_stmt_send_long_data);
SHINE_MYSQL_FUNC_DEF(mysql_stmt_result_metadata);
SHINE_MYSQL_FUNC_DEF(mysql_stmt_param_metadata);
SHINE_MYSQL_FUNC_DEF(mysql_stmt_errno);
SHINE_MYSQL_FUNC_DEF(mysql_stmt_error);
SHINE_MYSQL_FUNC_DEF(mysql_stmt_sqlstate);
SHINE_MYSQL_FUNC_DEF(mysql_stmt_row_seek);
SHINE_MYSQL_FUNC_DEF(mysql_stmt_row_tell);
SHINE_MYSQL_FUNC_DEF(mysql_stmt_data_seek);
SHINE_MYSQL_FUNC_DEF(mysql_stmt_num_rows);
SHINE_MYSQL_FUNC_DEF(mysql_stmt_affected_rows);
SHINE_MYSQL_FUNC_DEF(mysql_stmt_insert_id);
SHINE_MYSQL_FUNC_DEF(mysql_stmt_field_count);
SHINE_MYSQL_FUNC_DEF(mysql_commit);
SHINE_MYSQL_FUNC_DEF(mysql_rollback);
SHINE_MYSQL_FUNC_DEF(mysql_autocommit);
SHINE_MYSQL_FUNC_DEF(mysql_more_results);
SHINE_MYSQL_FUNC_DEF(mysql_next_result);
SHINE_MYSQL_FUNC_DEF(mysql_stmt_next_result);
SHINE_MYSQL_FUNC_DEF(mysql_close);

static SHINE_DLL_HANDLE mysql_dll_handle = NULL;

using namespace std;

namespace shine
{
    namespace db
    {

        class mysql_dll{
        public:
            // 程序退出释放动态加载的库
            static void unload()
            {
                if (mysql_dll_handle != NULL)
                {
                    dll::close(mysql_dll_handle);
                    mysql_dll_handle = NULL;
                }
            }

            // 动态加载 libmysql.dll 库
            static bool load()
            {
                if (mysql_dll_handle != NULL)
                    return true;

                const char* path;
#ifdef SHINE_OS_WINDOWS
                path = "libmysql.dll";
#else
                path = "libmysqlclient_r.so";
#endif
                mysql_dll_handle = dll::open(path);

                if (mysql_dll_handle == NULL)
                    return false;

                // 记录动态库路径，以便于在动态库卸载时输出库路径名
//                 __mysql_path = path;

/*
                __mysql_libversion = (mysql_libversion_fn) dll::sym(mysql_dll_handle, "mysql_get_client_version");
                if (__mysql_libversion == NULL)
                {
                    printf("load mysql_get_client_version from %s error: %d", path, dll::get_error());
                    return false;
                }

                __mysql_client_info = (mysql_client_info_fn) dll::sym(mysql_dll_handle, "mysql_get_client_info");
                if (__mysql_client_info == NULL)
                {
                    printf("load mysql_get_client_info from %s error: %d", path, dll::get_error());
                    return false;
                }

                __mysql_init = (mysql_init_fn) dll::sym(mysql_dll_handle, "mysql_init");
                if (__mysql_init == NULL)
                {
                    printf("load mysql_init from %s error: %d", path, dll::get_error());
                    return false;
                }

                __mysql_open = (mysql_open_fn) dll::sym(mysql_dll_handle, "mysql_real_connect");
                if (__mysql_open == NULL)
                {
                    printf("load mysql_real_connect from %s error: %d", path, dll::get_error());
                    return false;
                }

                __mysql_close = (mysql_close_fn) dll::sym(mysql_dll_handle, "mysql_close");
                if (__mysql_close == NULL)
                {
                    printf("load mysql_close from %s error: %d", path, dll::get_error());
                    return false;
                }

                __mysql_options = (mysql_options_fn) dll::sym(mysql_dll_handle, "mysql_options");
                if (__mysql_options == NULL)
                {
                    printf("load mysql_options from %s error: %d", path, dll::get_error());
                    return false;
                }

                __mysql_autocommit = (mysql_autocommit_fn) dll::sym(mysql_dll_handle, "mysql_autocommit");
                if (__mysql_autocommit == NULL)
                {
                    printf("load mysql_autocommit from %s error: %d", path, dll::get_error());
                    return false;
                }

                __mysql_errno = (mysql_errno_fn) dll::sym(mysql_dll_handle, "mysql_errno");
                if (__mysql_errno == NULL)
                {
                    printf("load mysql_errno from %s error: %d", path, dll::get_error());
                    return false;
                }

                __mysql_error = (mysql_error_fn) dll::sym(mysql_dll_handle, "mysql_error");
                if (__mysql_error == NULL)
                {
                    printf("load mysql_error from %s error: %d", path, dll::get_error());
                    return false;
                }

                __mysql_query = (mysql_query_fn) dll::sym(mysql_dll_handle, "mysql_query");
                if (__mysql_query == NULL)
                {
                    printf("load mysql_query from %s error: %d", path, dll::get_error());
                    return false;
                }

                __mysql_num_fields = (mysql_num_fields_fn) dll::sym(mysql_dll_handle, "mysql_num_fields");
                if (__mysql_num_fields == NULL)
                {
                    printf("load mysql_num_fields from %s error: %d", path, dll::get_error());
                    return false;
                }

                __mysql_fetch_fields = (mysql_fetch_fields_fn) dll::sym(mysql_dll_handle, "mysql_fetch_fields");
                if (__mysql_fetch_fields == NULL)
                {
                    printf("load mysql_fetch_fields from %s error: %d", path, dll::get_error());
                    return false;
                }

                __mysql_fetch_row = (mysql_fetch_row_fn) dll::sym(mysql_dll_handle, "mysql_fetch_row");
                if (__mysql_fetch_row == NULL)
                {
                    printf("load mysql_fetch_row from %s error: %d", path, dll::get_error());
                    return false;
                }

                __mysql_store_result = (mysql_store_result_fn) dll::sym(mysql_dll_handle, "mysql_store_result");
                if (__mysql_store_result == NULL)
                {
                    printf("load mysql_store_result from %s error: %d", path, dll::get_error());
                    return false;
                }

                __mysql_num_rows = (mysql_num_rows_fn) dll::sym(mysql_dll_handle, "mysql_num_rows");
                if (__mysql_num_rows == NULL)
                {
                    printf("load mysql_num_rows from %s error: %d", path, dll::get_error());
                    return false;
                }

                __mysql_free_result = (mysql_free_result_fn) dll::sym(mysql_dll_handle, "mysql_free_result");
                if (__mysql_free_result == NULL)
                {
                    printf("load mysql_free_result from %s error: %d", path, dll::get_error());
                    return false;
                }

                __mysql_affected_rows = (mysql_affected_rows_fn) dll::sym(mysql_dll_handle, "mysql_affected_rows");
                if (__mysql_affected_rows == NULL)
                {
                    printf("load mysql_affected_rows from %s error: %d", path, dll::get_error());
                    return false;
                }

                __mysql_set_character_set = (mysql_set_character_set_fn) dll::sym(mysql_dll_handle, "mysql_set_character_set");
                if (__mysql_affected_rows == NULL)
                {
                    printf("load mysql_set_character_set_fn %s error: %d", path, dll::get_error());
                    return false;
                }

                __mysql_character_set_name = (mysql_character_set_name_fn) dll::sym(mysql_dll_handle, "mysql_character_set_name");
                if (__mysql_affected_rows == NULL)
                {
                    printf("load mysql_character_set_name from %s error: %d", path, dll::get_error());
                    return false;
                }

                __mysql_thread_init = (mysql_thread_init_fn) dll::sym(mysql_dll_handle, "mysql_thread_init");
                if (__mysql_thread_init == NULL)
                {
                    printf("load mysql_thread_init from %s error: %d", path, dll::get_error());
                    return false;
                }

                __mysql_thread_end = (mysql_thread_end_fn) dll::sym(mysql_dll_handle, "mysql_thread_end");
                if (__mysql_thread_end == NULL)
                {
                    printf("load mysql_thread_end from %s error: %d", path, dll::get_error());
                    return false;
                }

                __mysql_server_init = (mysql_server_init_fn) dll::sym(mysql_dll_handle, "mysql_server_init");
                if (__mysql_server_init == NULL)
                { 
                    printf("load mysql_server_init from %s error: %d", path, dll::get_error());
                    return false;
                }

                __mysql_server_end = (mysql_server_end_fn) dll::sym(mysql_dll_handle, "mysql_server_end");
                if (__mysql_server_end == NULL)
                {
                    printf("load mysql_server_end from %s error: %d", path, dll::get_error());
                    return false;
                }
*/

                SHINE_MYSQL_FUNC_LOAD(mysql_get_parameters);
                SHINE_MYSQL_FUNC_LOAD(mysql_thread_init);
                SHINE_MYSQL_FUNC_LOAD(mysql_thread_end);
                SHINE_MYSQL_FUNC_LOAD(mysql_num_rows);
                SHINE_MYSQL_FUNC_LOAD(mysql_num_fields);
                SHINE_MYSQL_FUNC_LOAD(mysql_eof);
                SHINE_MYSQL_FUNC_LOAD(mysql_fetch_field_direct);
                SHINE_MYSQL_FUNC_LOAD(mysql_fetch_fields);
                SHINE_MYSQL_FUNC_LOAD(mysql_row_tell);
                SHINE_MYSQL_FUNC_LOAD(mysql_field_tell);
                SHINE_MYSQL_FUNC_LOAD(mysql_field_count);
                SHINE_MYSQL_FUNC_LOAD(mysql_affected_rows);
                SHINE_MYSQL_FUNC_LOAD(mysql_insert_id);
                SHINE_MYSQL_FUNC_LOAD(mysql_errno);
                SHINE_MYSQL_FUNC_LOAD(mysql_error);
                SHINE_MYSQL_FUNC_LOAD(mysql_sqlstate);
                SHINE_MYSQL_FUNC_LOAD(mysql_warning_count);
                SHINE_MYSQL_FUNC_LOAD(mysql_info);
                SHINE_MYSQL_FUNC_LOAD(mysql_thread_id);
                SHINE_MYSQL_FUNC_LOAD(mysql_character_set_name);
                SHINE_MYSQL_FUNC_LOAD(mysql_set_character_set);
                SHINE_MYSQL_FUNC_LOAD(mysql_init);
                SHINE_MYSQL_FUNC_LOAD(mysql_ssl_set);
                SHINE_MYSQL_FUNC_LOAD(mysql_get_ssl_cipher);
                SHINE_MYSQL_FUNC_LOAD(mysql_change_user);
                SHINE_MYSQL_FUNC_LOAD(mysql_real_connect);
                SHINE_MYSQL_FUNC_LOAD(mysql_select_db);
                SHINE_MYSQL_FUNC_LOAD(mysql_query);
                SHINE_MYSQL_FUNC_LOAD(mysql_send_query);
                SHINE_MYSQL_FUNC_LOAD(mysql_real_query);
                SHINE_MYSQL_FUNC_LOAD(mysql_store_result);
                SHINE_MYSQL_FUNC_LOAD(mysql_use_result);
                SHINE_MYSQL_FUNC_LOAD(mysql_get_character_set_info);
                SHINE_MYSQL_FUNC_LOAD(mysql_shutdown);
                SHINE_MYSQL_FUNC_LOAD(mysql_dump_debug_info);
                SHINE_MYSQL_FUNC_LOAD(mysql_refresh);
                SHINE_MYSQL_FUNC_LOAD(mysql_kill);
                SHINE_MYSQL_FUNC_LOAD(mysql_set_server_option);
                SHINE_MYSQL_FUNC_LOAD(mysql_ping);
                SHINE_MYSQL_FUNC_LOAD(mysql_stat);
                SHINE_MYSQL_FUNC_LOAD(mysql_get_server_info);
                SHINE_MYSQL_FUNC_LOAD(mysql_get_client_info);
                SHINE_MYSQL_FUNC_LOAD(mysql_get_client_version);
                SHINE_MYSQL_FUNC_LOAD(mysql_get_host_info);
                SHINE_MYSQL_FUNC_LOAD(mysql_get_server_version);
                SHINE_MYSQL_FUNC_LOAD(mysql_get_proto_info);
                SHINE_MYSQL_FUNC_LOAD(mysql_list_dbs);
                SHINE_MYSQL_FUNC_LOAD(mysql_list_tables);
                SHINE_MYSQL_FUNC_LOAD(mysql_list_processes);
                SHINE_MYSQL_FUNC_LOAD(mysql_options);
                SHINE_MYSQL_FUNC_LOAD(mysql_free_result);
                SHINE_MYSQL_FUNC_LOAD(mysql_data_seek);
                SHINE_MYSQL_FUNC_LOAD(mysql_row_seek);
                SHINE_MYSQL_FUNC_LOAD(mysql_field_seek);
                SHINE_MYSQL_FUNC_LOAD(mysql_fetch_row);
                SHINE_MYSQL_FUNC_LOAD(mysql_fetch_lengths);
                SHINE_MYSQL_FUNC_LOAD(mysql_fetch_field);
                SHINE_MYSQL_FUNC_LOAD(mysql_list_fields);
                SHINE_MYSQL_FUNC_LOAD(mysql_escape_string);
                SHINE_MYSQL_FUNC_LOAD(mysql_hex_string);
                SHINE_MYSQL_FUNC_LOAD(mysql_real_escape_string);
                SHINE_MYSQL_FUNC_LOAD(mysql_debug);
                SHINE_MYSQL_FUNC_LOAD(myodbc_remove_escape);
                SHINE_MYSQL_FUNC_LOAD(mysql_thread_safe);
                SHINE_MYSQL_FUNC_LOAD(mysql_embedded);
                SHINE_MYSQL_FUNC_LOAD(mysql_read_query_result);
                SHINE_MYSQL_FUNC_LOAD(mysql_stmt_init);
                SHINE_MYSQL_FUNC_LOAD(mysql_stmt_prepare);
                SHINE_MYSQL_FUNC_LOAD(mysql_stmt_execute);
                SHINE_MYSQL_FUNC_LOAD(mysql_stmt_fetch);
                SHINE_MYSQL_FUNC_LOAD(mysql_stmt_fetch_column);
                SHINE_MYSQL_FUNC_LOAD(mysql_stmt_store_result);
                SHINE_MYSQL_FUNC_LOAD(mysql_stmt_param_count);
                SHINE_MYSQL_FUNC_LOAD(mysql_stmt_attr_set);
                SHINE_MYSQL_FUNC_LOAD(mysql_stmt_attr_get);
                SHINE_MYSQL_FUNC_LOAD(mysql_stmt_bind_param);
                SHINE_MYSQL_FUNC_LOAD(mysql_stmt_bind_result);
                SHINE_MYSQL_FUNC_LOAD(mysql_stmt_close);
                SHINE_MYSQL_FUNC_LOAD(mysql_stmt_reset);
                SHINE_MYSQL_FUNC_LOAD(mysql_stmt_free_result);
                SHINE_MYSQL_FUNC_LOAD(mysql_stmt_send_long_data);
                SHINE_MYSQL_FUNC_LOAD(mysql_stmt_result_metadata);
                SHINE_MYSQL_FUNC_LOAD(mysql_stmt_param_metadata);
                SHINE_MYSQL_FUNC_LOAD(mysql_stmt_errno);
                SHINE_MYSQL_FUNC_LOAD(mysql_stmt_error);
                SHINE_MYSQL_FUNC_LOAD(mysql_stmt_sqlstate);
                SHINE_MYSQL_FUNC_LOAD(mysql_stmt_row_seek);
                SHINE_MYSQL_FUNC_LOAD(mysql_stmt_row_tell);
                SHINE_MYSQL_FUNC_LOAD(mysql_stmt_data_seek);
                SHINE_MYSQL_FUNC_LOAD(mysql_stmt_num_rows);
                SHINE_MYSQL_FUNC_LOAD(mysql_stmt_affected_rows);
                SHINE_MYSQL_FUNC_LOAD(mysql_stmt_insert_id);
                SHINE_MYSQL_FUNC_LOAD(mysql_stmt_field_count);
                SHINE_MYSQL_FUNC_LOAD(mysql_commit);
                SHINE_MYSQL_FUNC_LOAD(mysql_rollback);
                SHINE_MYSQL_FUNC_LOAD(mysql_autocommit);
                SHINE_MYSQL_FUNC_LOAD(mysql_more_results);
                SHINE_MYSQL_FUNC_LOAD(mysql_next_result);
                SHINE_MYSQL_FUNC_LOAD(mysql_stmt_next_result);
                SHINE_MYSQL_FUNC_LOAD(mysql_close);

                return true;
            }

        };

    }//namesapce db
}

