//
// Created by heweiyu on 2021/1/24.
//

#include "HttpConn.h"

#include <cassert>
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>

#include "utils.h"

std::string HttpConn::m_srcDir;
std::atomic<int> HttpConn::userCount;

HttpConn::HttpConn()
    : m_fd(-1), m_addr(sockaddr_in{0}), m_isClose(false)
    {}

HttpConn::~HttpConn() {
    Close();
}

void HttpConn::init(int fd, const sockaddr_in &addr) {
    assert(fd > 0);
//    std::cout << "client add fd = " << fd << std::endl;
    m_fd = fd;
    m_addr = addr;
    m_isClose = false;
    m_readBuffer.clear();
    m_writeBuffer.clear();
    userCount++;
}

void HttpConn::Close() {
    m_response.UnmapFile();
    if(!m_isClose) {
        m_isClose = true;
        userCount--;
        close(m_fd);
//        std::cout << "client close fd = " << m_fd << std::endl;
    }
}

int HttpConn::GetFd() const {
    return m_fd;
}

int HttpConn::GetPort() const {
    return m_addr.sin_port;
}

const char * HttpConn::GetIP() const {
    return inet_ntoa(m_addr.sin_addr);
}

sockaddr_in HttpConn::GetAddr() const {
    return m_addr;
}

bool HttpConn::read() {
    ssize_t len = readn(m_fd, m_readBuffer);
    if(len <= 0) {
        return false;
    }
    return true;
}

int HttpConn::write(int *saveErrno) {
    ssize_t len = -1;
    do {
        len = writev(m_fd, m_iov, m_iovCnt);
        if(len <= 0) { // 出错返回
            *saveErrno = errno;
            break;
        }
        if(m_iov[0].iov_len + m_iov[1].iov_len  == 0) { break; } /* 传输结束 */
        else if(static_cast<size_t>(len) > m_iov[0].iov_len) {
            // 头部发送完成，移动 m_iov[1].iov_base(即文件)指针 offset = len - m_iov[0].iov_len
            m_iov[1].iov_base = (uint8_t*) m_iov[1].iov_base + (len - m_iov[0].iov_len);
            m_iov[1].iov_len -= (len - m_iov[0].iov_len); // 更新 m_iov[1].iov_len
            if(m_iov[0].iov_len) { // 设置 m_iov[0].iov_len = 0 下次不再重新发送
                m_iov[0].iov_len = 0;
                m_writeBuffer.clear();
            }
        }
        else {
            // 头部未发送完成，更新 offset
            m_iov[0].iov_base = (uint8_t*)m_iov[0].iov_base + len;
            m_iov[0].iov_len -= len;
            m_writeBuffer = m_writeBuffer.substr(len);
        }
    } while(ToWriteBytes() > 10240);
    return len;
}

// 对 m_readBuffer 中的数据进行解析
bool HttpConn::process() {
    m_request.init();
    if(m_readBuffer.empty()) {
        return false;
    }
    if (m_request.parse(m_readBuffer)) {
        // 解析成功
        m_response.Init(m_srcDir, m_request.path(), m_request.isKeepAlice(), 200);
    }else {
        // 解析失败
        m_response.Init(m_srcDir, m_request.path(), false, 400);
    }
    m_readBuffer.clear();
    m_response.MakeResponse(m_writeBuffer);
    /*响应头*/
    m_iov[0].iov_base = m_writeBuffer.data();
    m_iov[0].iov_len = m_writeBuffer.size();
    m_iovCnt = 1;
    /*文件*/
    if(m_response.File() && m_response.FileSize() > 0) {
        m_iov[1].iov_base = m_response.File();
        m_iov[1].iov_len = m_response.FileSize();
        m_iovCnt = 2;
    }
    return true;
}


