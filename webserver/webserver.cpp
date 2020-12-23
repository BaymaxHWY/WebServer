//
// Created by heweiyu on 2020/12/21.
//
#include "webserver.h"

WebServer::WebServer() : m_port(9999), m_thread_pool_num(4), m_is_end(false), m_events(MAXEVENTNUM) {
    m_pool = new ThreadPool(m_thread_pool_num, MAXTASKNUM);
}

WebServer::~WebServer() {
    // todo: 释放资源
    delete m_pool;
}

void WebServer::Init(int port, int thread_pool_num) {
    m_port = port;
    m_thread_pool_num = thread_pool_num;
}

void WebServer::EventListen() {
    // create socket
    m_listen_fd = Socket(AF_INET, SOCK_STREAM, 0);

    // 设置可重用本地址（强制使用处于 TIME_WAIT 的 socket 地址）
    int reuse = 1;
    Setsockopt(m_listen_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    // bind socket
    sockaddr_in hint{};
    // memset(&hint, 0, sizeof(hint));
    hint.sin_family = AF_INET;
    hint.sin_addr.s_addr = htonl(INADDR_ANY);
    hint.sin_port = htons(m_port);
    Bind(m_listen_fd, (sockaddr*)& hint, sizeof(hint));

    // listen socket
    Listen(m_listen_fd, 5);

    // epoll 注册内核事件表（水平触发）
    m_epoll_fd = Epoll_create(5);
    epoll_add(m_epoll_fd, m_listen_fd, false); // m_listen_fd 阻塞 IO
}

void WebServer::EventLoop() {
    while (!m_is_end) {
        // accpet socket
        // m_events.begin() 的返回值是一个 iterator，*it 得到值（即 epoll_event 类型的值），再对其取地址
        // (todo 问题？vector 是怎么实现的？ 这样取到的指针就是 epoll_event 数组的第一个)
        int event_num = Epoll_wait(m_epoll_fd, &(*m_events.begin()), m_events.size(), -1);
        // 事件分发
        for(int i = 0; i < event_num; i++) {
            int fd = m_events[i].data.fd; // 取到就绪 fd
            if(fd == m_listen_fd) {
                // 有新的 client 连接
                sockaddr_in client_hint{};
                socklen_t client_len = sizeof(client_hint);
                int conn_fd = Accept(fd, (sockaddr*)& client_hint, &client_len);
                // 加入 epoll 内核事件表(设置边缘触发，oneshot)
                epoll_add(m_epoll_fd, conn_fd, true);
                // 设置非阻塞 fd（边缘触发需要设置 fd 为非阻塞）
                setnoblockfd(conn_fd);
                //为这个 client 创建 HttpConn 服务，添加到 m_conns 中
                m_conns[conn_fd] = new HttpConn();
            }else if(m_events[i].events & (EPOLLIN | EPOLLOUT)) {
                // 有数据读/写
                // 使用信号量表示目前待处理 task 数量，线程池中的线程通过信号量被唤醒
                // 一个存储 connect socket 的 task 队列，使用锁(lock)实现跨线程之间的同步(master 和 worker，worker 之间)
                // 加入任务队列
                m_pool->task_append(m_conns[fd]);
            }else if(m_events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                // todo: 服务器关闭连接
                close(fd);
                epoll_del(m_epoll_fd, fd);
            }
        }
    }
}