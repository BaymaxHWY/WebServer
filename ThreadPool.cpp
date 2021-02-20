//
// Created by heweiyu on 2021/1/24.
//

#include "ThreadPool.h"
#include <cassert>
#include <pthread.h>

ThreadPool::ThreadPool(size_t threadCount)
    : m_thread_count(threadCount),
      m_task_queue(),
      m_threads(threadCount),
      m_close(false)
{
    assert(m_thread_count > 0);
    for(size_t i = 0; i < m_thread_count; i++) {
        pthread_create(&m_threads[i], nullptr, workerThread, this);
    }
}

void ThreadPool::AddTask(ThreadPool::Functor&& func)
{
    m_task_queue.put(func);
}

void* ThreadPool::workerThread(void* arg)
{
    pthread_detach(pthread_self());
    auto pool = static_cast<ThreadPool*>(arg);
    while (!pool->m_close) {
        auto task = pool->m_task_queue.take();
        task();
    }
    return nullptr;
}

ThreadPool::~ThreadPool() {
    m_close = true;
}
