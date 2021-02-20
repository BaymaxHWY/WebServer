//
// Created by heweiyu on 2021/1/24.
//

#ifndef WEBSERVER_THREADPOOL_H
#define WEBSERVER_THREADPOOL_H

#include <functional>
#include <vector>
#include <pthread.h>

#include "base/noncopyable.h"
#include "base/BlockingQueue.h"

class ThreadPool : private noncopyable {
public:
    typedef std::function<void()> Functor;
    explicit ThreadPool(size_t threadCount = 1);
    ~ThreadPool();
    void AddTask(Functor&&);

    static void* workerThread(void*);
private:
    // 线程数
    size_t m_thread_count;
    // 线程安全的阻塞任务队列（生产者消费者模式）
    BlockingQueue<Functor> m_task_queue;
    // 线程
    std::vector<pthread_t> m_threads;
    bool m_close;
};


#endif //WEBSERVER_THREADPOOL_H
