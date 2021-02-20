//
// Created by heweiyu on 2021/2/20.
//

#include "HeapTimer.h"
#include <cassert>
#include <algorithm>
#include <iostream>

void HeapTimer::siftUp(size_t i) {
    assert(i >= 0 && i < m_heap.size());
    if(i == 0) return;
    int cur = i;
    // 与父节点比较，如果不满足最小堆性质则交换，同时更新 cur, father
    int father = (cur - 1) / 2;
    while(father >= 0) {
        if(m_heap[father] < m_heap[cur]) break;
        swapNode(father, cur);
        cur = father;
        father = (cur - 1) / 2;
    }
}

void HeapTimer::siftDown(size_t i) {
    assert(i >= 0 && i < m_heap.size());
    // 与两个子节点（如果存在）比较
    size_t max_node = i;
    size_t left = i * 2 + 1;
    size_t right = i * 2 + 2;
    if(left < m_heap.size() && m_heap[left] < m_heap[max_node])
        max_node = left;
    if(right < m_heap.size() && m_heap[right] < m_heap[max_node])
        max_node = right;

    if(max_node != i) {
        swapNode(i, max_node);
        siftDown(max_node);
    }
}

void HeapTimer::swapNode(size_t i, size_t j) {
    if(i == j) return;
    assert(i >= 0 && i < m_heap.size());
    assert(j >= 0 && j < m_heap.size());
    // 需要交换 m_heap 的值和 m_ref 的映射关系
    std::swap(m_heap[i], m_heap[j]);
    m_ref[m_heap[i].id] = i;
    m_ref[m_heap[j].id] = j;
}

void HeapTimer::add(int id, int timeOut, const TimeOutCallBack &cb) {
    assert(id >= 0);
    size_t i;
    if(m_ref.count(id) == 0) {
        /* 新节点：堆尾插入，调整堆 */
        i = m_heap.size();
        m_ref[id] = i;
        m_heap.push_back({id, Clock::now() + MS(timeOut), cb});
        siftUp(i);
    }else {
        /* 已有结点：调整堆 */
        i = m_ref[id];
        m_heap[i].expires = Clock::now() + MS(timeOut);
        m_heap[i].cb = cb;
        // 这两个操作最多只会执行一个
        siftDown(i);
        siftUp(i);
    }
}

void HeapTimer::delNode(int id) {
    /* 删除指定id结点，并触发回调函数 */
    if(m_heap.empty() || m_ref.count(id) == 0) {
        return;
    }
    size_t i;
    i = m_ref[id];
    TimerNode node = m_heap[i];
    node.cb();
    del(i);
//    std::cout << "timer delete id = " << id << std::endl;
}

void HeapTimer::del(size_t i) {
    assert(i >= 0 && i < m_heap.size());
    // 删除 m_heap[i] 的节点
    size_t tail = m_heap.size() - 1;
    if(i < tail) {
        swapNode(i, tail);
        siftUp(i);
        siftDown(i);
    }
    m_ref.erase(m_heap[tail].id);
    m_heap.pop_back();
}

void HeapTimer::adjust(int id, int newExpires) {
    /* 调整指定id的结点 */
    assert(!m_heap.empty() && m_ref.count(id) != 0);
    size_t i = m_ref[id];
    m_heap[i].expires = Clock::now() + MS(newExpires);
    siftUp(i);
    siftDown(i);
}

void HeapTimer::clear() {
    m_heap.clear();
    m_ref.clear();
}

int HeapTimer::GetNextTick() {
    // 检查超时，return 下一个即将超时的时间(int 类型表示 ms 的基本单位供 epoll_wait time out 使用)
    tick();
    int res = -1;
    if(!m_heap.empty()) {
        res = std::chrono::duration_cast<MS>(m_heap.front().expires - Clock::now()).count();
        if(res < 0) res = 0; // 如果这时已经存在超时的节点设置为 0
    }
    return res; // res = -1 表示 heap 中没有节点
}

void HeapTimer::tick() {
    /* 清除超时结点 */
    while(!m_heap.empty()) {
        TimerNode node = m_heap.front();
        if(!isExpired(node.expires)) {
            // 没有超时
            break;
        }
        delNode(node.id);
    }
}

bool HeapTimer::isExpired(const TimeStamp &expires) const {
    return std::chrono::duration_cast<MS>(expires - Clock::now()).count() <= 0;
}