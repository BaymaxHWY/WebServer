//
// Created by heweiyu on 2021/1/24.
//

#ifndef WEBSERVER_WEBSERVER_H
#define WEBSERVER_WEBSERVER_H

#include <memory>
#include <unordered_map>

//#include "Epoll.h"
//#include "HttpConn.h"
//#include "ThreadPool.h"

class HttpConn;
class Epoll;
class ThreadPool;
class HeapTimer;

class WebServer {
public:
    WebServer(int port, int thread_num, int timeOutMS);
    ~WebServer();

    void start();
private:
    bool initSocket();
    void accpetConnect();
    // 为什么传指针而不是 &，因为 bind 的参数默认是 copy or move，如果用 std::ref() 可以使用 &
    void closeClient(std::shared_ptr<HttpConn>);
    void dealRead(std::shared_ptr<HttpConn>);
    void dealWrite(std::shared_ptr<HttpConn>);
    void onRead(std::shared_ptr<HttpConn>);
    void onWrite(std::shared_ptr<HttpConn>);
    void onProcess(std::shared_ptr<HttpConn>);

    void timerAdjust(std::shared_ptr<HttpConn>);
private:
    int m_port;
    int m_thread_num;
    bool m_isClose;
    int m_listenfd;
    int m_timeoutMS;

    std::unique_ptr<Epoll> m_epoll;
    std::unique_ptr<ThreadPool> m_pool;
    std::unique_ptr<HeapTimer> m_timer;
    std::unordered_map<int, std::shared_ptr<HttpConn>> m_clients;
};


#endif //WEBSERVER_WEBSERVER_H
