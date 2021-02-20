//
// Created by heweiyu on 2021/1/31.
//

#ifndef WEBSERVER_BLOCKINGQUEUE_H
#define WEBSERVER_BLOCKINGQUEUE_H

#include "noncopyable.h"
#include "Mutex.h"

#include <deque>
#include <memory>
#include <cassert>

template<typename T>
class BlockingQueue : noncopyable
{
public:
    BlockingQueue()
        : m_mutex(),
          m_cond(m_mutex),
          m_queue()
    {}

    void put(const T& x)
    {
        MutexLockGuard lock(m_mutex);
        m_queue.push_back(x);
        m_cond.notify();
    }

    void put(T&& x)
    {
        MutexLockGuard lock(m_mutex);
        m_queue.push_back(std::move(x));
        m_cond.notify();
    }

    T take()
    {
        MutexLockGuard lock(m_mutex);
        // always use a while-loop, due to spurious wakeup
        while (m_queue.empty())
        {
            m_cond.wait();
        }
        assert(!m_queue.empty());
        T res(std::move(m_queue.front()));
        m_queue.pop_front();
        return res;
    }

    size_t size() const
    {
        MutexLockGuard lock(m_mutex);
        return m_queue.size();
    }
private:
    mutable MutexLock m_mutex;
    Condition m_cond;
    std::deque<T> m_queue;
};

#endif //WEBSERVER_BLOCKINGQUEUE_H
