 /**
 *****************************************************************************
 *
 *@note shineframe¿ª·¢¿ò¼Ü https://github.com/shineframe/shineframe
 *
 *@file zeromq.hpp
 *
 *@brief zmq·â×°
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
#include "zeromq_helper.hpp"

#if (defined SHINE_OS_WINDOWS)
#else
#endif


using namespace std;

namespace shine
{
    class zeromq
    {
    public:     
        static void setup()
        {
            static once_flag t;
            std::call_once(t, []{
                zeromq_dll::load();
            });
        }

        static int error_no() {        
            return zmq_errno_func();
        }

        static const char * error_info(int errnum){
            return zmq_strerror_func(errnum);
        }

        static void version(int *major, int *minor, int *patch){
            return zmq_version_func(major, minor, patch);
        }

        static void * ctx_new(){
            return zmq_ctx_new_func();
        }

        static int ctx_term(void *context){
            return zmq_ctx_term_func(context);
        }

        static int ctx_shutdown(void *ctx_){
            return zmq_ctx_shutdown_func(ctx_);
        }

        static int ctx_set(void *context, int option, int optval){
            return zmq_ctx_set_func(context, option, optval);
        }
        
        static int ctx_get(void *context, int option){
            return zmq_ctx_get_func(context, option);
        }

        static void * init(int io_threads){
            return zmq_init_func(io_threads);
        }

        static int term(void *context){
            return zmq_term_func(context);
        }

        static int ctx_destroy(void *context){
            return zmq_ctx_destroy_func(context);
        }

        static int msg_init(zmq_msg_t *msg){
            return zmq_msg_init_func(msg);
        }
        
        static int msg_init_size(zmq_msg_t *msg, size_t size){
            return zmq_msg_init_size_func(msg, size);
        }

        static int msg_init_data(zmq_msg_t *msg, void *data, size_t size, zmq_free_fn *ffn, void *hint){
            return zmq_msg_init_data_func(msg, data, size, ffn, hint);
        }

        static int msg_send(zmq_msg_t *msg, void *s, int flags){
            return zmq_msg_send_func(msg, s, flags);
        }

        static int msg_recv(zmq_msg_t *msg, void *s, int flags){
            return zmq_msg_recv_func(msg, s, flags);
        }

        static int msg_close(zmq_msg_t *msg){
            return zmq_msg_close_func(msg);
        }

        static int msg_move(zmq_msg_t *dest, zmq_msg_t *src){
            return zmq_msg_move_func(dest, src);
        }

        static int msg_copy(zmq_msg_t *dest, zmq_msg_t *src){
            return zmq_msg_copy_func(dest, src);
        }

        static void *msg_data(zmq_msg_t *msg){
            return zmq_msg_data_func(msg);
        }

        static size_t msg_size(zmq_msg_t *msg){
            return zmq_msg_size_func(msg);
        }

        static int msg_more(zmq_msg_t *msg){
            return zmq_msg_more_func(msg);
        }

        static int msg_get(zmq_msg_t *msg, int property){
            return zmq_msg_get_func(msg, property);
        }

        static int msg_set(zmq_msg_t *msg, int property, int optval){
            return zmq_msg_set_func(msg, property, optval);
        }

        static const char * msg_gets(zmq_msg_t *msg, const char *property){
            return zmq_msg_gets_func(msg, property);
        }

        static void * socket(void *ctx, int type){
            return zmq_socket_func(ctx, type);
        }

        static int close(void *s){
            return zmq_close_func(s);
        }

        static int setsockopt(void *s, int option, const void *optval, size_t optvallen){
            return zmq_setsockopt_func(s, option, optval, optvallen);
        }

        static int getsockopt(void *s, int option, void *optval, size_t *optvallen){
            return zmq_getsockopt_func(s, option, optval, optvallen);
        }

        static int bind(void *s, const char *addr){
            return zmq_bind_func(s, addr);
        }

        static int connect(void *s, const char *addr){
            return zmq_connect_func(s, addr);
        }

        static int unbind(void *s, const char *addr){
            return zmq_unbind_func(s, addr);
        }

        static int disconnect(void *s, const char *addr){
            return zmq_disconnect_func(s, addr);
        }

        static int send(void *s, const void *buf, size_t len, int flags){
            return zmq_send_func(s, buf, len, flags);
        }
        
        static int send_const(void *s, const void *buf, size_t len, int flags){
            return zmq_send_const_func(s, buf, len, flags);
        }

        static int recv(void *s, void *buf, size_t len, int flags){
            return zmq_recv_func(s, buf, len, flags);
        }

        static int socket_monitor(void *s, const char *addr, int events){
            return zmq_socket_monitor_func(s, addr, events);
        }

        static int poll(zmq_pollitem_t *items, int nitems, long timeout){
            return zmq_poll_func(items, nitems, timeout);
        }

        static int proxy(void *frontend, void *backend, void *capture){
            return zmq_proxy_func(frontend, backend, capture);
        }

        static int proxy_steerable(void *frontend, void *backend, void *capture, void *control){
            return zmq_proxy_steerable_func(frontend, backend, capture, control);
        }

        static int has(const char *capability){
            return zmq_has_func(capability);
        }

        static int device(int type, void *frontend, void *backend){
            return zmq_device_func(type, frontend, backend);
        }

        static int sendmsg(void *s, zmq_msg_t *msg, int flags){
            return zmq_sendmsg_func(s, msg, flags);
        }
        
        static int recvmsg(void *s, zmq_msg_t *msg, int flags){
            return zmq_recvmsg_func(s, msg, flags);
        }

        static char *  z85_encode(char *dest, const uint8_t *data, size_t size){
            return zmq_z85_encode_func(dest, data, size);
        }

        static  uint8_t *  z85_decode_t(uint8_t *dest, const char *string){
            return zmq_z85_decode_func(dest, string);
        }

        static int curve_keypair(char *z85_public_key, char *z85_secret_key){
            return zmq_curve_keypair_func(z85_public_key, z85_secret_key);
        }

        static int sendiov(void *s, struct iovec *iov, size_t count, int flags){
            return zmq_sendiov_func(s, iov, count, flags);
        }

        static int recviov(void *s, struct iovec *iov, size_t *count, int flags){
            return zmq_recviov_func(s, iov, count, flags);
        }

        static void *  stopwatch_start(){
            return zmq_stopwatch_start_func();
        }

        static unsigned long stopwatch_stop(void *watch_){
            return zmq_stopwatch_stop_func(watch_);
        }

        static void sleep(int seconds_){
            return zmq_sleep_func(seconds_);
        }

        static void * threadstart_t(zmq_thread_fn* func, void* arg){
            return zmq_threadstart_func(func, arg);
        }

        static void threadclose(void* thread){
            return zmq_threadclose_func(thread);
        }


    public:

    };
}
