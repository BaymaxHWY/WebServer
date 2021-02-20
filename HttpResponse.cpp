//
// Created by heweiyu on 2021/1/30.
//

#include "HttpResponse.h"

#include <cassert>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>

const std::unordered_map<std::string, std::string> HttpResponse::SUFFIX_TYPE = {
        { ".html",  "text/html" },
        { ".xml",   "text/xml" },
        { ".xhtml", "application/xhtml+xml" },
        { ".txt",   "text/plain" },
        { ".rtf",   "application/rtf" },
        { ".pdf",   "application/pdf" },
        { ".word",  "application/msword" },
        { ".png",   "image/png" },
        { ".gif",   "image/gif" },
        { ".jpg",   "image/jpeg" },
        { ".jpeg",  "image/jpeg" },
        { ".au",    "audio/basic" },
        { ".mpeg",  "video/mpeg" },
        { ".mpg",   "video/mpeg" },
        { ".avi",   "video/x-msvideo" },
        { ".gz",    "application/x-gzip" },
        { ".tar",   "application/x-tar" },
        { ".css",   "text/css "},
        { ".js",    "text/javascript "},
};

/*other code to describe*/
const std::unordered_map<int, std::string> HttpResponse::CODE_STATUS = {
        { 200, "OK" },
        { 400, "Bad Request" },
        { 403, "Forbidden" },
        { 404, "Not Found" },
};

/*error html code to path*/
const std::unordered_map<int, std::string> HttpResponse::CODE_PATH = {
        { 400, "/400.html" },
        { 403, "/403.html" },
        { 404, "/404.html" },
};

HttpResponse::HttpResponse() {
    m_code = -1;
    m_path = m_srcDir = "";
    m_isKeepAlive = false;
    m_mmFile = nullptr;
}

HttpResponse::~HttpResponse() {
    UnmapFile();
}

void HttpResponse::Init(const std::string &srcDir, const std::string &path,
                        bool isKeepAlive, int code) {
    assert(srcDir != "");
    if(m_mmFile) UnmapFile();
    m_code = code;
    m_isKeepAlive = isKeepAlive;
    m_srcDir = srcDir;
    m_path = path;
    m_mmFile = nullptr;
    m_mmFileStat = {};
}

void HttpResponse::MakeResponse(std::string &buffer) {
    /* 判断请求的资源文件 */
//    std::cout << "path : " << m_srcDir + m_path << std::endl;
    if(stat((m_srcDir + m_path).c_str(), &m_mmFileStat) < 0
            || S_ISDIR(m_mmFileStat.st_mode)) {
        m_code = 404; // 文件不存在或者是一个目录
    }
    else if(!(m_mmFileStat.st_mode & S_IROTH)) { // S_IROTH ： Read by others.
        m_code = 403; // 无权限读取文件
    }
    else if(m_code == -1) {
        m_code = 200;
    }
    errorHtml();
    addStateLine(buffer);
    addHeaders(buffer);
    addContent(buffer);
}

char * HttpResponse::File() {
    return m_mmFile;
}

size_t HttpResponse::FileSize() const {
    return m_mmFileStat.st_size;
}

void HttpResponse::errorHtml() {
    if(CODE_PATH.count(m_code)) {
        m_path = CODE_PATH.find(m_code)->second;
        stat((m_srcDir + m_path).c_str(), &m_mmFileStat);
    }
}
/*
HTTP/1.1 200 OK
Connection: Keep-Alive
Content-Encoding: gzip
Content-Type: text/html; charset=utf-8
Date: Thu, 11 Aug 2016 15:23:13 GMT
Keep-Alive: timeout=5, max=1000
Last-Modified: Mon, 25 Jul 2016 04:32:39 GMT
Server: Apache
 */
void HttpResponse::addStateLine(std::string &buffer) {
    std::string status;
    if(CODE_STATUS.count(m_code)) {
        status = CODE_STATUS.find(m_code)->second;
    }else {
        /*请求无效(Bad request) 400*/
        status = CODE_STATUS.find(400)->second;
    }
    buffer += "HTTP/1.1 " + std::to_string(m_code) + " " + status + " \r\n";
}

void HttpResponse::addHeaders(std::string &buffer) {
    buffer += "Connection: ";
    if(m_isKeepAlive) {
        buffer += "keep-alive\r\n";
        // todo Keep-Alive 头部 允许消息发送者暗示连接的状态，还可以用来设置超时时长和最大请求数。
    }else {
        buffer += "close\r\n";
    }
    buffer += "Content-type: " + getFileType() + "\r\n";
}

void HttpResponse::addContent(std::string &buffer) {
    int fileFd = open((m_srcDir + m_path).c_str(), O_RDONLY); // 只读获取文件描述符
    if(fileFd < 0) {
        ErrorContent(buffer, "File Not Found!");
        return;
    }
    /* 将文件映射到内存提高文件的访问速度
        MAP_PRIVATE 建立一个写入时拷贝的私有映射，对其他进程不可见*/
    m_mmFile = (char*)mmap(0, m_mmFileStat.st_size, PROT_READ, MAP_PRIVATE, fileFd, 0);
    if(m_mmFile == MAP_FAILED) {
        ErrorContent(buffer, "File Not Found!");
        return;
    }
    close(fileFd); // 关闭文件描述符
    buffer += "Content-length: " + std::to_string(m_mmFileStat.st_size) + "\r\n\r\n";
}

void HttpResponse::UnmapFile() {
    if(m_mmFile) {
        munmap(m_mmFile, m_mmFileStat.st_size);
        m_mmFile = nullptr;
    }
}

std::string HttpResponse::getFileType() {
    /* 判断文件类型 */
    auto pos = m_path.find_last_of(".");
    if(pos == std::string::npos) {
        return "text/plain";
    }
    std::string suffix = m_path.substr(pos);
    if(SUFFIX_TYPE.count(suffix) == 1) {
        return SUFFIX_TYPE.find(suffix)->second;
    }
    return "text/plain";
}

void HttpResponse::ErrorContent(std::string &buffer, const std::string &msg) {
    std::string body, status;
    body += "<html><title>Error</title>";
    body += "<body bgcolor=\"ffffff\">";
    if(CODE_STATUS.count(m_code) == 1) {
        status = CODE_STATUS.find(m_code)->second;
    } else {
        status = "Bad Request";
    }
    body += std::to_string(m_code) + " : " + status  + "\n";
    body += "<p>" + msg + "</p>";
    body += "<hr><em>BY BayMax Server</em></body></html>";
    buffer += "Content-length: " + std::to_string(body.size()) + "\r\n\r\n";
    buffer += body;
}