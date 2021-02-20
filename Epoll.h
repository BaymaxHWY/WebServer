//
// Created by heweiyu on 2021/1/24.
//

#ifndef WEBSERVER_EPOLL_H
#define WEBSERVER_EPOLL_H

#include <sys/epoll.h>
#include <vector>

class Epoll {
public:
    explicit Epoll(int max_events = 1024);
    ~Epoll();

    bool AddFd(int fd, uint32_t events);
    bool ModFd(int fd, uint32_t events);
    bool DelFd(int fd);

    int Wait(int timeout = -1);
    int GetFd(size_t i) const;
    uint32_t GetEvents(size_t i) const;

private:
    int m_epollfd;
    std::vector<epoll_event> m_events;
};


#endif //WEBSERVER_EPOLL_H
