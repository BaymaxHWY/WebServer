//
// Created by heweiyu on 2021/1/24.
//

#include "Epoll.h"
#include <unistd.h>
#include <cassert>

Epoll::Epoll(int max_event) : m_epollfd(epoll_create(1)), m_events(max_event) {
    assert(m_epollfd >= 0 && m_events.size() > 0);
}

Epoll::~Epoll() {
    close(m_epollfd);
}

bool Epoll::AddFd(int fd, uint32_t events) {
    if(fd < 0) return false;
    epoll_event e{};
    e.data.fd = fd;
    e.events = events;
    return epoll_ctl(m_epollfd, EPOLL_CTL_ADD, fd, &e) == 0;
}

bool Epoll::ModFd(int fd, uint32_t events) {
    if(fd < 0) return false;
    epoll_event e{};
    e.data.fd = fd;
    e.events = events;
    return epoll_ctl(m_epollfd, EPOLL_CTL_MOD, fd, &e) == 0;
}

bool Epoll::DelFd(int fd) {
    if(fd < 0) return false;
    epoll_event e{};
    e.data.fd = fd;
    return epoll_ctl(m_epollfd, EPOLL_CTL_DEL, fd, &e) == 0;
}

int Epoll::Wait(int timeout) {
    return epoll_wait(m_epollfd, &m_events[0], static_cast<int>(m_events.size()), timeout);
}

int Epoll::GetFd(size_t i) const {
    assert(i < m_events.size() && i >= 0);
    return m_events[i].data.fd;
}

uint32_t Epoll::GetEvents(size_t i) const {
    assert(i < m_events.size() && i >= 0);
    return m_events[i].events;
}
