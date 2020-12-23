//
// Created by heweiyu on 2020/12/21.
//

#ifndef WEBSERVER_THREADPOOL_H
#define WEBSERVER_THREADPOOL_H

#include <vector>
#include <queue>
#include <pthread.h>
#include <semaphore.h>
#include <cstdio>

#include "../http/http_conn.h"
#include "../util/util.h"

class ThreadPool {
public:
    explicit ThreadPool() = default;
    explicit ThreadPool(int max_thread_nums, int max_task_nums);
    ~ThreadPool();

    void task_append(HttpConn* conn);

private:
    static void* worker(void* args);
    void run();

private:
    // 维护一个 task queue（todo task 的类型？）
    std::queue<HttpConn*> tasks_;
    // 维护一个 thread pool
    std::vector<pthread_t> pool_;
    // 保护 task 队列的 lock
    pthread_mutex_t mu_;
    // 用来唤醒工作线程 sem
    sem_t sem_;
    bool is_end;

    int max_thread_nums_;
    int max_task_nums_;
};

#endif //WEBSERVER_THREADPOOL_H
