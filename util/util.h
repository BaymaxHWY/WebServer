//
// Created by heweiyu on 2020/12/21.
//

#ifndef WEBSERVER_UTIL_H
#define WEBSERVER_UTIL_H

#include <unistd.h>
#include <fcntl.h>
#include <cstdio>
#include <cerrno>
#include <cstdlib>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <pthread.h>

/* 输出 msg: error，并 exit */
void sys_error(const char* msg);

/* socket 系统调用封装 */
int Socket(int domain, int type, int protocol);
void Setsockopt(int sockfd, int level, int optname, const void *optval, int optlen);
void Bind(int sockfd, struct sockaddr *my_addr, int addrlen);
void Listen(int sockfd, int backlog);
int Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);

/* epoll 系统调用封装 */
int Epoll_create(int size);
int Epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);
void Epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);

/* pthread 系统调用封装 */
void Pthread_create(pthread_t *tidp, pthread_attr_t *attrp,
                    void * (*routine)(void *), void *argp);
//void Pthread_join(pthread_t tid, void **thread_return);
//void Pthread_cancel(pthread_t tid);
void Pthread_detach(pthread_t tid);
//void Pthread_exit(void *retval);
//pthread_t Pthread_self(void);
//void Pthread_once(pthread_once_t *once_control, void (*init_function)());

/* epoll 相关操作封装函数 */
void epoll_add(int epoll_fd, int fd, bool mode);
void epoll_del(int epoll_fd, int fd);
void epoll_mod(int epoll_fd, int fd, int options);

// 设置非阻塞 fd
int setnoblockfd(int fd);

#endif //WEBSERVER_UTIL_H
