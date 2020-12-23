//
// Created by heweiyu on 2020/12/21.
//

#ifndef WEBSERVER_WEBSERVER_H
#define WEBSERVER_WEBSERVER_H

#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <cassert>
#include <cstring>
#include <vector>
#include <unistd.h>
#include <unordered_map>

#include "../util/util.h"
#include "../threadpool/threadpool.h"
#include "../http/http_conn.h"

const int MAXEVENTNUM = 5000;
const int MAXTASKNUM = 2000;

class WebServer { // 作为 master thread
    // 1. socket 初始化、监听，epoll IO 复用
    // 2. 初始化 threadpool
    // 3. accpet 监听 listen socket，将 connect socket 放入 tasks queue 并唤醒一个工作线程（如何唤醒？一次唤醒全部还是一个？）
public:
    explicit WebServer();
    ~WebServer();

    void Init(int port, int thread_pool_num); // 配置初始化参数
    void EventListen(); // 初始化 socket，生成 listen socket
    void EventLoop(); // 主要逻辑

private:
    int m_port; // 端口号
    int m_listen_fd; // listen socket fd
    int m_epoll_fd; // epoll_fd
    std::vector<epoll_event> m_events; // 传入 epoll_wait 中的事件数组
    std::unordered_map<int, HttpConn*> m_conns; // 存储每个 client 的 connect socket 对应的 HttpConn 服务

    int m_thread_pool_num; // 线程池大小
    ThreadPool* m_pool; // 线程池指针

    bool m_is_end;
};

#endif //WEBSERVER_WEBSERVER_H
