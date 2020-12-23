//
// Created by heweiyu on 2020/12/21.
//
#include "util.h"

void sys_error(const char* msg) {
    perror(msg);
    exit(-1);
}

int Socket(int domain, int type, int protocol) {
    int fd = socket(domain, type, protocol);
    if(fd == -1)
        sys_error("Socket error");
    return fd;
}

void Setsockopt(int sockfd, int level, int optname, const void *optval, int optlen) {
    if (setsockopt(sockfd, level, optname, optval, optlen) < 0)
        sys_error("Setsockopt error");
}

void Bind(int sockfd, struct sockaddr *my_addr, int addrlen) {
    if(bind(sockfd,my_addr, addrlen) < 0)
        sys_error("Bind error");
}

void Listen(int sockfd, int backlog) {
    if(listen(sockfd, backlog) < 0)
        sys_error("Listen error");
}

int Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
    int rc;
    if((rc = accept(sockfd, addr, addrlen)) < 0)
        sys_error("Accept error");
    return rc;
}

int Epoll_create(int size) {
    int rc;
    if((rc = epoll_create(size)) < 0)
        sys_error("Epoll_create error");
    return rc;
}

int Epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout) {
    int rc;
    if((rc = epoll_wait(epfd, events, maxevents, timeout)) < 0)
        sys_error("Epoll_wait error");
    return rc;
}

void Epoll_ctl(int epfd, int op, int fd, struct epoll_event *event) {
    if(epoll_ctl(epfd, op, fd, event) < 0)
        sys_error("Epoll_ctl error");
}

void Pthread_create(pthread_t *tidp, pthread_attr_t *attrp,
                    void * (*routine)(void *), void *argp) {
    if(pthread_create(tidp, attrp, routine, argp) != 0)
        sys_error("Pthread_create error");
}

void Pthread_detach(pthread_t tid) {
    if(pthread_detach(tid) != 0)
        sys_error("Pthread_detach error");
}

void epoll_add(int epoll_fd, int fd, bool mode) {
    epoll_event event{};
    event.data.fd = fd;
    if(mode) {
        event.events = EPOLLIN | EPOLLOUT | EPOLLET | EPOLLONESHOT;
    }else {
        event.events = EPOLLIN;
    }
    Epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);
}

void epoll_del(int epoll_fd, int fd) {
    Epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
}

void epoll_mod(int epoll_fd, int fd, int options) {
    epoll_event event{};
    event.data.fd = fd;
    event.events = options;
    Epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &event);
}

int setnoblockfd(int fd) {
    int old_options = fcntl(fd, F_GETFL);
    int new_options = old_options | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_options);
    return old_options;
}