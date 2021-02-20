//
// Created by heweiyu on 2021/1/24.
//

#include "WebServer.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <cassert>
#include <functional>
#include <cstring>

#include "utils.h"
#include "HttpConn.h"
#include "ThreadPool.h"
#include "Epoll.h"
#include "HeapTimer.h"

const int ConnEvent = EPOLLHUP | EPOLLRDHUP | EPOLLET | EPOLLONESHOT;

WebServer::WebServer(int port, int thread_num, int timeOutMS) :
    m_port(port),
    m_thread_num(thread_num),
    m_isClose(false),
    m_timeoutMS(timeOutMS),
    m_epoll(std::make_unique<Epoll>()),
    m_pool(std::make_unique<ThreadPool>(thread_num)),
    m_timer(std::make_unique<HeapTimer>())
{
    auto src_dir = getcwd(nullptr, 256);
    strncat(src_dir, "/resources", 16);
    HttpConn::m_srcDir = src_dir;
    if(!initSocket()) {m_isClose = true;}
}

WebServer::~WebServer() {
    close(m_listenfd);
}

void WebServer::start() {
    int time_ms = -1;
    if(!m_isClose) std::cout << "========== server start ===========" <<std::endl;
    while(!m_isClose) {
        if(m_timeoutMS > 0) {
            // 获取下一个超时时间 todo 定时器
            time_ms = m_timer->GetNextTick();
        }
        int n = m_epoll->Wait(time_ms);
//        std::cout << "get epoll wait" << std::endl;
        for(int i = 0; i < n; i++) {
            /*处理事件*/
            int fd = m_epoll->GetFd(i);
            uint32_t events = m_epoll->GetEvents(i);
            if(fd == m_listenfd) {
                accpetConnect();
            }else if(events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                assert(m_clients.count(fd) > 0);
                closeClient(m_clients[fd]);
            }else if(events & EPOLLIN) {
//                if(m_clients.count(fd) == 0) {
//                    std::cout << "EPOLLIN " << fd << std::endl;
//                }
//                std::cout << "EPOLLIN " << fd << std::endl;
                assert(m_clients.count(fd) > 0);
                dealRead(m_clients[fd]);
            }else if(events & EPOLLOUT) {
//                if(m_clients.count(fd) == 0) {
//                    std::cout << "EPOLLOUT " << fd << std::endl;
//                }
//                std::cout << "EPOLLOUT " << fd << std::endl;
                assert(m_clients.count(fd) > 0);
                dealWrite(m_clients[fd]);
            }
        }
    }
}

/* Create listenFd */
bool WebServer::initSocket() {
    if(m_port > 65535 || m_port < 1024) {
        std::cout << "port is out of range" << std::endl;
        return false;
    }
    m_listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(m_listenfd < 0) {
        std::cout << "socket creat error" << std::endl;
        return false;
    }
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(m_port);

    int ret;
    int on = 1;
    ret = setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    if(ret < 0) {
        std::cout << "setsockopt SO_REUSEADDR error" << std::endl;
        return false;
    }
    setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on));
    if(ret < 0) {
        std::cout << "setsockopt SO_REUSEPORT error" << std::endl;
        return false;
    }

    if(bind(m_listenfd, (sockaddr*) &server_addr, sizeof(server_addr)) < 0) {
        std::cout << "bind error" << std::endl;
        return false;
    }

    ret = listen(m_listenfd, 50);
    if(ret < 0) {
        std::cout << "listen error" << std::endl;
        return false;
    }
    std::cout << "server listen port : " << m_port << std::endl;
    m_epoll->AddFd(m_listenfd, EPOLLIN | EPOLLHUP | EPOLLRDHUP);
    return true;
}

void WebServer::accpetConnect() {
    sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int newfd = accept(m_listenfd, (sockaddr*) &client_addr, &addr_len);
    if(newfd < 0) return;
    if(!m_clients[newfd])
        m_clients[newfd] = std::make_shared<HttpConn>();
    m_clients[newfd]->init(newfd, client_addr);
    // todo 加入定时器
    if(m_timeoutMS > 0) {
        m_timer->add(newfd, m_timeoutMS, std::bind(&WebServer::closeClient, this, m_clients[newfd]));
    }
    m_epoll->AddFd(newfd, ConnEvent | EPOLLIN);
    setNoBlockFd(newfd);
//    std::cout << "client in!" << std::endl;
}

void WebServer::closeClient(std::shared_ptr<HttpConn> conn) {
    assert(conn);
    m_epoll->DelFd(conn->GetFd());
    conn->Close();
//    m_clients.erase(conn->GetFd()); 不需要去销毁，可以复用 httpConn 结构
//    std::cout << "client remove " << conn->GetFd() << std::endl;
}

void WebServer::dealRead(std::shared_ptr<HttpConn> conn) {
    assert(conn);
    timerAdjust(conn); // 重置超时时间
    m_pool->AddTask(std::bind(&WebServer::onRead, this, conn));
}

void WebServer::dealWrite(std::shared_ptr<HttpConn> conn) {
    assert(conn);
    timerAdjust(conn); // 重置超时时间
    m_pool->AddTask(std::bind(&WebServer::onWrite, this, conn));
}

void WebServer::timerAdjust(std::shared_ptr<HttpConn> conn) {
    assert(conn);
    if(m_timeoutMS > 0) {
        m_timer->adjust(conn->GetFd(), m_timeoutMS);
    }
}

void WebServer::onRead(std::shared_ptr<HttpConn> conn) {
    assert(conn);
    if (!conn->read()) {
       closeClient(conn);
       return;
    }
    onProcess(conn);
}

void WebServer::onWrite(std::shared_ptr<HttpConn> conn) {
    assert(conn);
    int ret = -1;
    int writeErrno = 0;
    ret = conn->write(&writeErrno);
    if(conn->ToWriteBytes() == 0) {
        /*传输完成*/
        if(conn->isKeepAlive()) {
            onProcess(conn);
            return;
        }
    }
    else if(ret < 0) {
        if(writeErrno == EAGAIN) {
            /* 继续传输 */
            m_epoll->ModFd(conn->GetFd(), ConnEvent | EPOLLOUT);
            return;
        }
    }
    closeClient(conn);
}

void WebServer::onProcess(std::shared_ptr<HttpConn> conn) {
    if(conn->process()) {
        m_epoll->ModFd(conn->GetFd(), ConnEvent | EPOLLOUT); // 等待去发送
    }else {
        m_epoll->ModFd(conn->GetFd(), ConnEvent | EPOLLIN); // 等待继续读
    }
}

