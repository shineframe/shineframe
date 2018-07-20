 /**
 *****************************************************************************
 *
 *@note shineframe开发框架 https://github.com/shineframe/shineframe
 *
 *@file zeromq_helper.hpp
 *
 *@brief zmq铺助
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
#include "zmq/zmq.h"


#if (defined SHINE_OS_WINDOWS)
#else
#endif

typedef int(*zmq_errno_t) (void);
typedef const char *(*zmq_strerror_t) (int errnum);
typedef void(*zmq_version_t) (int *major, int *minor, int *patch);
typedef void * (*zmq_ctx_new_t) (void);
typedef int(*zmq_ctx_term_t) (void *context);
typedef int(*zmq_ctx_shutdown_t) (void *ctx_);
typedef int(*zmq_ctx_set_t) (void *context, int option, int optval);
typedef int(*zmq_ctx_get_t) (void *context, int option);
typedef void * (*zmq_init_t) (int io_threads);
typedef int(*zmq_term_t) (void *context);
typedef int(*zmq_ctx_destroy_t) (void *context);
typedef int(*zmq_msg_init_t) (zmq_msg_t *msg);
typedef int(*zmq_msg_init_size_t) (zmq_msg_t *msg, size_t size);
typedef int(*zmq_msg_init_data_t) (zmq_msg_t *msg, void *data, size_t size, zmq_free_fn *ffn, void *hint);
typedef int(*zmq_msg_send_t) (zmq_msg_t *msg, void *s, int flags);
typedef int(*zmq_msg_recv_t) (zmq_msg_t *msg, void *s, int flags);
typedef int(*zmq_msg_close_t) (zmq_msg_t *msg);
typedef int(*zmq_msg_move_t) (zmq_msg_t *dest, zmq_msg_t *src);
typedef int(*zmq_msg_copy_t) (zmq_msg_t *dest, zmq_msg_t *src);
typedef void * (*zmq_msg_data_t) (zmq_msg_t *msg);
typedef size_t(*zmq_msg_size_t) (zmq_msg_t *msg);
typedef int(*zmq_msg_more_t) (zmq_msg_t *msg);
typedef int(*zmq_msg_get_t) (zmq_msg_t *msg, int property);
typedef int(*zmq_msg_set_t) (zmq_msg_t *msg, int property, int optval);
typedef const char *(*zmq_msg_gets_t) (zmq_msg_t *msg, const char *property);
typedef void * (*zmq_socket_t) (void *, int type);
typedef int(*zmq_close_t) (void *s);
typedef int(*zmq_setsockopt_t) (void *s, int option, const void *optval, size_t optvallen);
typedef int(*zmq_getsockopt_t) (void *s, int option, void *optval, size_t *optvallen);
typedef int(*zmq_bind_t) (void *s, const char *addr);
typedef int(*zmq_connect_t) (void *s, const char *addr);
typedef int(*zmq_unbind_t) (void *s, const char *addr);
typedef int(*zmq_disconnect_t) (void *s, const char *addr);
typedef int(*zmq_send_t) (void *s, const void *buf, size_t len, int flags);
typedef int(*zmq_send_const_t) (void *s, const void *buf, size_t len, int flags);
typedef int(*zmq_recv_t) (void *s, void *buf, size_t len, int flags);
typedef int(*zmq_socket_monitor_t) (void *s, const char *addr, int events);
typedef int(*zmq_poll_t) (zmq_pollitem_t *items, int nitems, long timeout);
typedef int(*zmq_proxy_t) (void *frontend, void *backend, void *capture);
typedef int(*zmq_proxy_steerable_t) (void *frontend, void *backend, void *capture, void *control);
typedef int(*zmq_has_t) (const char *capability);
typedef int(*zmq_device_t) (int type, void *frontend, void *backend);
typedef int(*zmq_sendmsg_t) (void *s, zmq_msg_t *msg, int flags);
typedef int(*zmq_recvmsg_t) (void *s, zmq_msg_t *msg, int flags);
typedef char *  (*zmq_z85_encode_t) (char *dest, const uint8_t *data, size_t size);
typedef  uint8_t *  (*zmq_z85_decode_t) (uint8_t *dest, const char *string);
typedef int(*zmq_curve_keypair_t) (char *z85_public_key, char *z85_secret_key);
typedef int(*zmq_sendiov_t) (void *s, struct iovec *iov, size_t count, int flags);
typedef int(*zmq_recviov_t) (void *s, struct iovec *iov, size_t *count, int flags);
typedef void * (*zmq_stopwatch_start_t) (void);
typedef  unsigned long(*zmq_stopwatch_stop_t) (void *watch_);
typedef void(*zmq_sleep_t) (int seconds_);
typedef void * (*zmq_threadstart_t) (zmq_thread_fn* func, void* arg);
typedef void(*zmq_threadclose_t) (void* thread);


#define SHINE_ZEROMQ_FUNC_DEF(name) static name##_t name##_func = NULL;
#define SHINE_ZEROMQ_FUNC_LOAD(name) { name##_func = (name##_t) dll::sym(zmq_dll_handle, #name);\
if (name##_func == NULL)\
{\
    printf("load %s error: %d", #name, dll::get_error());\
    return false;\
}\
}

SHINE_ZEROMQ_FUNC_DEF(zmq_errno);
SHINE_ZEROMQ_FUNC_DEF(zmq_strerror);
SHINE_ZEROMQ_FUNC_DEF(zmq_version);
SHINE_ZEROMQ_FUNC_DEF(zmq_ctx_new);
SHINE_ZEROMQ_FUNC_DEF(zmq_ctx_term);
SHINE_ZEROMQ_FUNC_DEF(zmq_ctx_shutdown);
SHINE_ZEROMQ_FUNC_DEF(zmq_ctx_set);
SHINE_ZEROMQ_FUNC_DEF(zmq_ctx_get);
SHINE_ZEROMQ_FUNC_DEF(zmq_init);
SHINE_ZEROMQ_FUNC_DEF(zmq_term);
SHINE_ZEROMQ_FUNC_DEF(zmq_ctx_destroy);
SHINE_ZEROMQ_FUNC_DEF(zmq_msg_init);
SHINE_ZEROMQ_FUNC_DEF(zmq_msg_init_size);
SHINE_ZEROMQ_FUNC_DEF(zmq_msg_init_data);
SHINE_ZEROMQ_FUNC_DEF(zmq_msg_send);
SHINE_ZEROMQ_FUNC_DEF(zmq_msg_recv);
SHINE_ZEROMQ_FUNC_DEF(zmq_msg_close);
SHINE_ZEROMQ_FUNC_DEF(zmq_msg_move);
SHINE_ZEROMQ_FUNC_DEF(zmq_msg_copy);
SHINE_ZEROMQ_FUNC_DEF(zmq_msg_data);
SHINE_ZEROMQ_FUNC_DEF(zmq_msg_size);
SHINE_ZEROMQ_FUNC_DEF(zmq_msg_more);
SHINE_ZEROMQ_FUNC_DEF(zmq_msg_get);
SHINE_ZEROMQ_FUNC_DEF(zmq_msg_set);
SHINE_ZEROMQ_FUNC_DEF(zmq_msg_gets);
SHINE_ZEROMQ_FUNC_DEF(zmq_socket);
SHINE_ZEROMQ_FUNC_DEF(zmq_close);
SHINE_ZEROMQ_FUNC_DEF(zmq_setsockopt);
SHINE_ZEROMQ_FUNC_DEF(zmq_getsockopt);
SHINE_ZEROMQ_FUNC_DEF(zmq_bind);
SHINE_ZEROMQ_FUNC_DEF(zmq_connect);
SHINE_ZEROMQ_FUNC_DEF(zmq_unbind);
SHINE_ZEROMQ_FUNC_DEF(zmq_disconnect);
SHINE_ZEROMQ_FUNC_DEF(zmq_send);
SHINE_ZEROMQ_FUNC_DEF(zmq_send_const);
SHINE_ZEROMQ_FUNC_DEF(zmq_recv);
SHINE_ZEROMQ_FUNC_DEF(zmq_socket_monitor);
SHINE_ZEROMQ_FUNC_DEF(zmq_poll);
SHINE_ZEROMQ_FUNC_DEF(zmq_proxy);
SHINE_ZEROMQ_FUNC_DEF(zmq_proxy_steerable);
SHINE_ZEROMQ_FUNC_DEF(zmq_has);
SHINE_ZEROMQ_FUNC_DEF(zmq_device);
SHINE_ZEROMQ_FUNC_DEF(zmq_sendmsg);
SHINE_ZEROMQ_FUNC_DEF(zmq_recvmsg);
SHINE_ZEROMQ_FUNC_DEF(zmq_z85_encode);
SHINE_ZEROMQ_FUNC_DEF(zmq_z85_decode);
SHINE_ZEROMQ_FUNC_DEF(zmq_curve_keypair);
SHINE_ZEROMQ_FUNC_DEF(zmq_sendiov);
SHINE_ZEROMQ_FUNC_DEF(zmq_recviov);
SHINE_ZEROMQ_FUNC_DEF(zmq_stopwatch_start);
SHINE_ZEROMQ_FUNC_DEF(zmq_stopwatch_stop);
SHINE_ZEROMQ_FUNC_DEF(zmq_sleep);
SHINE_ZEROMQ_FUNC_DEF(zmq_threadstart);
SHINE_ZEROMQ_FUNC_DEF(zmq_threadclose);


static SHINE_DLL_HANDLE zmq_dll_handle = NULL;

using namespace std;

namespace shine
{
    class zeromq_dll{
    public:
        // 程序退出释放动态加载的库
        static void unload()
        {
            if (zmq_dll_handle != NULL)
            {
                dll::close(zmq_dll_handle);
                zmq_dll_handle = NULL;
            }
        }

        // 动态加载 libmysql.dll 库
        static bool load()
        {
            if (zmq_dll_handle != NULL)
                return true;

            const char* path;
#ifdef SHINE_OS_WINDOWS
            path = "libzmq.dll";
#else
            path = "libzmq.so";
#endif
            zmq_dll_handle = dll::open(path);

            if (zmq_dll_handle == NULL)
                return false;

            SHINE_ZEROMQ_FUNC_LOAD(zmq_errno);
            SHINE_ZEROMQ_FUNC_LOAD(zmq_strerror);
            SHINE_ZEROMQ_FUNC_LOAD(zmq_version);
            SHINE_ZEROMQ_FUNC_LOAD(zmq_ctx_new);
            SHINE_ZEROMQ_FUNC_LOAD(zmq_ctx_term);
            SHINE_ZEROMQ_FUNC_LOAD(zmq_ctx_shutdown);
            SHINE_ZEROMQ_FUNC_LOAD(zmq_ctx_set);
            SHINE_ZEROMQ_FUNC_LOAD(zmq_ctx_get);
            SHINE_ZEROMQ_FUNC_LOAD(zmq_init);
            SHINE_ZEROMQ_FUNC_LOAD(zmq_term);
            SHINE_ZEROMQ_FUNC_LOAD(zmq_ctx_destroy);
            SHINE_ZEROMQ_FUNC_LOAD(zmq_msg_init);
            SHINE_ZEROMQ_FUNC_LOAD(zmq_msg_init_size);
            SHINE_ZEROMQ_FUNC_LOAD(zmq_msg_init_data);
            SHINE_ZEROMQ_FUNC_LOAD(zmq_msg_send);
            SHINE_ZEROMQ_FUNC_LOAD(zmq_msg_recv);
            SHINE_ZEROMQ_FUNC_LOAD(zmq_msg_close);
            SHINE_ZEROMQ_FUNC_LOAD(zmq_msg_move);
            SHINE_ZEROMQ_FUNC_LOAD(zmq_msg_copy);
            SHINE_ZEROMQ_FUNC_LOAD(zmq_msg_data);
            SHINE_ZEROMQ_FUNC_LOAD(zmq_msg_size);
            SHINE_ZEROMQ_FUNC_LOAD(zmq_msg_more);
            SHINE_ZEROMQ_FUNC_LOAD(zmq_msg_get);
            SHINE_ZEROMQ_FUNC_LOAD(zmq_msg_set);
            SHINE_ZEROMQ_FUNC_LOAD(zmq_msg_gets);
            SHINE_ZEROMQ_FUNC_LOAD(zmq_socket);
            SHINE_ZEROMQ_FUNC_LOAD(zmq_close);
            SHINE_ZEROMQ_FUNC_LOAD(zmq_setsockopt);
            SHINE_ZEROMQ_FUNC_LOAD(zmq_getsockopt);
            SHINE_ZEROMQ_FUNC_LOAD(zmq_bind);
            SHINE_ZEROMQ_FUNC_LOAD(zmq_connect);
            SHINE_ZEROMQ_FUNC_LOAD(zmq_unbind);
            SHINE_ZEROMQ_FUNC_LOAD(zmq_disconnect);
            SHINE_ZEROMQ_FUNC_LOAD(zmq_send);
            SHINE_ZEROMQ_FUNC_LOAD(zmq_send_const);
            SHINE_ZEROMQ_FUNC_LOAD(zmq_recv);
            SHINE_ZEROMQ_FUNC_LOAD(zmq_socket_monitor);
            SHINE_ZEROMQ_FUNC_LOAD(zmq_poll);
            SHINE_ZEROMQ_FUNC_LOAD(zmq_proxy);
            SHINE_ZEROMQ_FUNC_LOAD(zmq_proxy_steerable);
            SHINE_ZEROMQ_FUNC_LOAD(zmq_has);
            SHINE_ZEROMQ_FUNC_LOAD(zmq_device);
            SHINE_ZEROMQ_FUNC_LOAD(zmq_sendmsg);
            SHINE_ZEROMQ_FUNC_LOAD(zmq_recvmsg);
            SHINE_ZEROMQ_FUNC_LOAD(zmq_z85_encode);
            SHINE_ZEROMQ_FUNC_LOAD(zmq_z85_decode);
            SHINE_ZEROMQ_FUNC_LOAD(zmq_curve_keypair);
            SHINE_ZEROMQ_FUNC_LOAD(zmq_sendiov);
            SHINE_ZEROMQ_FUNC_LOAD(zmq_recviov);
            SHINE_ZEROMQ_FUNC_LOAD(zmq_stopwatch_start);
            SHINE_ZEROMQ_FUNC_LOAD(zmq_stopwatch_stop);
            SHINE_ZEROMQ_FUNC_LOAD(zmq_sleep);
            SHINE_ZEROMQ_FUNC_LOAD(zmq_threadstart);
            SHINE_ZEROMQ_FUNC_LOAD(zmq_threadclose);


            return true;
        }

    };

}

