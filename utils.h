//
// Created by heweiyu on 2021/1/24.
//

#ifndef WEBSERVER_UTILS_H
#define WEBSERVER_UTILS_H

#include <fcntl.h>
#include <unistd.h>
#include <string>

int setNoBlockFd(int fd);

ssize_t readn(int fd, std::string &inBuffer);

#endif //WEBSERVER_UTILS_H
