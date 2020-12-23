//
// Created by heweiyu on 2020/12/21.
//

#include "threadpool.h"

ThreadPool::ThreadPool(int max_thread_nums, int max_task_nums)
            : max_thread_nums_(max_task_nums),
              max_task_nums_(max_task_nums),
              is_end(false) {
    // 初始化线程池
    pool_.resize(max_thread_nums);
    for(int i = 0; i < max_thread_nums; i++) {
        Pthread_create(&pool_[i], nullptr, worker, this);
        // 设置线程状态是 detached，线程运行结束后线程拥有的内存会自动被系统回收（否则需要显示调用 pthread_join 回收）
        Pthread_detach(pool_[i]);
    }
    // todo 初始化任务队列
    // 初始化锁、信号量
    sem_init(&sem_, 0, 0);
    pthread_mutex_init(&mu_, nullptr);
}

ThreadPool::~ThreadPool() {
    sem_destroy(&sem_);
    pthread_mutex_destroy(&mu_);
    is_end = true;
}


void* ThreadPool::worker(void* args) {
    auto* thread = static_cast<ThreadPool*>(args);
    thread->run();
    return thread;
}

void ThreadPool::run() {
    // 工作线程逻辑
    while (!is_end) {
        // 1. 等待任务
        sem_wait(&sem_);
        // 2. 获取 task 队列的操作互斥锁
        pthread_mutex_lock(&mu_);
        // 3. 取得任务，并将其从任务队列中剔除(tasks_队列可能为空吗？)
        HttpConn *conn = tasks_.front();
        tasks_.pop();
        pthread_mutex_unlock(&mu_);
        // 交给 HttpConn 处理业务逻辑（socket 的读写也交给了 httpconn class）
        conn->process();
    }
}

void ThreadPool::task_append(HttpConn* conn) {
    // 将 conn 加入任务队列
    // 1、获取互斥锁
    // 2、添加进 task 队列中
    // 3、增加 sem
    pthread_mutex_lock(&mu_);
    tasks_.push(conn);
    pthread_mutex_unlock(&mu_);
    sem_post(&sem_);
}

