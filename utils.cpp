//
// Created by heweiyu on 2021/1/31.
//
#include "utils.h"

const int MAX_BUF = 4096;

int setNoBlockFd(int fd) {
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
}

ssize_t readn(int fd, std::string &inBuffer) {
    ssize_t nread = 0;
    ssize_t readSum = 0;
    inBuffer.clear();
    while(true) {
        char buff[MAX_BUF];
        if((nread = read(fd, buff, MAX_BUF)) < 0) {
            if(errno == EINTR) { // 被信号中断继续读
                continue;
            }else if(errno == EAGAIN || errno == EWOULDBLOCK) { // 已经读完了
                return readSum;
            }else { // 错误
                return -1;
            }
        }else if(nread == 0) {
            break;
        }

        readSum += nread;
        inBuffer += std::string(buff, buff + nread);
    }
    return readSum;
}