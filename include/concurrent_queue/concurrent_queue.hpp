#pragma once

#include "cameron314/concurrentqueue.h"
#include "cameron314/blockingconcurrentqueue.h"

namespace shine {
    template<typename T> 
    class concurrent_queue{
    public:
        bool push(const T &obj){
            return _queue.enqueue(obj);
        }

        bool push(T &&obj){
            return _queue.enqueue(obj);
        }

        template<typename ARR>
        bool push_bulk(ARR arr, std::size_t count){
            return _queue.enqueue_bulk(arr, count);
        }

        bool pop(T &obj){
            return _queue.try_dequeue(obj);
        }

        template<typename ARR>
        std::size_t pop_bulk(ARR arr, std::size_t count){
            return _queue.try_dequeue_bulk(arr, count);
        }

        std::size_t size_approx() const {
            return _queue.size_approx();
        }

    private:
        moodycamel::ConcurrentQueue<T> _queue;
    };

    template<typename T>
    class block_concurrent_queue{
    public:
        bool push(const T &obj){
            return _queue.enqueue(obj);
        }

        bool push(T &&obj){
            return _queue.enqueue(obj);
        }

        template<typename ARR>
        bool push_bulk(ARR arr, std::size_t count){
            return _queue.enqueue_bulk(arr, count);
        }

        void pop(T &obj){
            _queue.wait_dequeue(obj);
        }

        template<typename ARR>
        std::size_t pop_bulk(ARR arr, std::size_t count){
            return _queue.wait_dequeue_bulk(arr, count);
        }

        std::size_t size_approx() const {
            return _queue.size_approx();
        }

        void wait_pop(T &obj){
            return _queue.wait_dequeue(obj);
        }

        bool wait_pop_timed(T &obj, std::int64_t timeout_usecs){
            return _queue.wait_dequeue_timed(obj, timeout_usecs);
        }

        template<typename ARR>
        std::size_t wait_pop_bulk(ARR arr, std::size_t count){
            return _queue.wait_dequeue_bulk(arr, count);
        }

        template<typename ARR>
        std::size_t wait_pop_bulk_timed(ARR arr, std::size_t count, std::int64_t timeout_usecs){
            return _queue.wait_dequeue_bulk_timed(arr, count, timeout_usecs);
        }

    private:
        moodycamel::BlockingConcurrentQueue<T> _queue;
    };

}

