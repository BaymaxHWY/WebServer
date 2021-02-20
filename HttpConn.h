//
// Created by heweiyu on 2021/1/24.
//

#ifndef WEBSERVER_HTTPCONN_H
#define WEBSERVER_HTTPCONN_H

#include <netinet/in.h>
#include <string>
#include <sys/uio.h> // writev
#include <atomic>

#include "HttpRequest.h"
#include "HttpResponse.h"

class HttpConn {
public:
    HttpConn();
    ~HttpConn();

    void init(int fd, const sockaddr_in& addr);


    bool read();
    int write(int *saveErrno);

    void Close();

    int GetFd() const;

    int GetPort() const;

    const char* GetIP() const;

    sockaddr_in GetAddr() const;

    bool process();

    int ToWriteBytes() {
        return m_iov[0].iov_len + m_iov[1].iov_len;
    }

    bool isKeepAlive() const {
        return m_request.isKeepAlice();
    }

    static std::string m_srcDir; // todo 设置好 html 的地址，放几个页面
    static std::atomic<int> userCount; // 记录所有的连接数量

private:
    int m_fd;
    sockaddr_in m_addr;
    bool m_isClose;
    std::string m_readBuffer;
    std::string m_writeBuffer;
    // todo 实现一个 atomic int

    /*用于 writev 函数*/
    int m_iovCnt;
    struct iovec m_iov[2];

    HttpRequest m_request;
    HttpResponse m_response;
};


#endif //WEBSERVER_HTTPCONN_H
