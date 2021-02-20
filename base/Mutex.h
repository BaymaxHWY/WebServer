//
// Created by heweiyu on 2021/1/31.
//

#ifndef WEBSERVER_MUTEXLOCK_H
#define WEBSERVER_MUTEXLOCK_H

#include "noncopyable.h"
#include <pthread.h>

class MutexLock : private noncopyable {
public:
    MutexLock() {
        pthread_mutex_init(&m_mutex, nullptr);
    }
    ~MutexLock() {
        pthread_mutex_destroy(&m_mutex);
    }

    void lock() {
        pthread_mutex_lock(&m_mutex);
    }
    void unlock() {
        pthread_mutex_unlock(&m_mutex);
    }

    pthread_mutex_t* getMutex() {
        return &m_mutex;
    }
private:
    pthread_mutex_t m_mutex;
};

class MutexLockGuard : noncopyable {
public:
    explicit MutexLockGuard(MutexLock& mutex) : m_mutex(mutex)
    {
        m_mutex.lock();
    }
    ~MutexLockGuard() {
        m_mutex.unlock();
    }

private:
    MutexLock& m_mutex;
};

class Condition : private noncopyable
{
public:
    explicit Condition(MutexLock& mutex) : m_mutex(mutex)
    {
        pthread_cond_init(&m_cond, nullptr);
    }

    ~Condition()
    {
        pthread_cond_destroy(&m_cond);
    }

    void wait()
    {
        pthread_cond_wait(&m_cond, m_mutex.getMutex());
    }

    void notify()
    {
        pthread_cond_signal(&m_cond);
    }

    void notifyAll()
    {
        pthread_cond_broadcast(&m_cond);
    }

private:
    pthread_cond_t m_cond;
    MutexLock& m_mutex;
};

#define MutexLockGuard(x) static_assert(false, "missing mutex guard var name")

#endif //WEBSERVER_MUTEXLOCK_H
