//
// Created by heweiyu on 2021/2/20.
//

#ifndef WEBSERVER_HEAPTIMER_H
#define WEBSERVER_HEAPTIMER_H

#include <chrono>
#include <functional>
#include <vector>
#include <unordered_map>

typedef std::function<void()> TimeOutCallBack;
typedef std::chrono::high_resolution_clock Clock; // Clock用于获取系统时间
typedef std::chrono::milliseconds MS; // 用于时间计算的基本单位
typedef Clock::time_point TimeStamp; // 某一时刻的时间戳

// 最小堆上的timer节点
struct TimerNode {
    int id;
    TimeStamp expires;
    TimeOutCallBack cb;
    bool operator < (const TimerNode& rhs) const {
        return expires < rhs.expires;
    }
};


class HeapTimer {
public:
    HeapTimer() {m_heap.reserve(64);}

    ~HeapTimer() {clear();}

    void adjust(int id, int newExpires);

    void add(int id, int timeOut, const TimeOutCallBack& cb);

    int GetNextTick();

private:
    void clear();

    void delNode(int id);

    void del(size_t i);
    // 节点 i 向上调整
    void siftUp(size_t i);
    // 节点 i 向下调整
    void siftDown(size_t i);

    void swapNode(size_t i, size_t j);

    bool isExpired(const TimeStamp& expires) const;

    void tick();

private:
    std::vector<TimerNode> m_heap; // vector 存储堆
    std::unordered_map<int, size_t> m_ref; // 存储 TimerNode.id 到 m_heap 下标的映射
};


#endif //WEBSERVER_HEAPTIMER_H
