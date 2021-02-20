//
// Created by heweiyu on 2021/1/25.
//

#include "HttpRequest.h"
#include <iostream>

//const std::unordered_set<std::string> HttpRequest::DEFAULT_HTML{
//        "/index", "/register", "/login",
//        "/welcome", "/video", "/picture", };
//
//const std::unordered_map<std::string, int> HttpRequest::DEFAULT_HTML_TAG {
//        {"/register.html", 0}, {"/login.html", 1},  };

void HttpRequest::init() {
    m_method = m_path = "";
    m_state = REQUEST_LINE;
    m_headers.clear();
    m_posts.clear();
}

// return 是否解析成功
bool HttpRequest::parse(std::string& buffer) {
    const std::string CRLF = "\r\n";
    if(buffer.empty())
        return false;
    size_t idx = 0;
    while(idx < buffer.size() && m_state != FINISH) {
        size_t found = buffer.find(CRLF, idx);
        if(found == std::string::npos) {
            if(m_state != BODY)
                return false;
            else {
                found = buffer.size();
            }
        }
        std::string line = buffer.substr(idx, found - idx);
        switch (m_state) {
            case REQUEST_LINE:
                if(!parseRequestLine(line)) {
                    return false;
                }
                parsePath();
                break;
            case HEADERS:
                parseHeader(line);
                break;
            case BODY:
                parseBody(line);
                break;
            default:
                break;
        }
        idx = found + CRLF.size();
    }
    return true;
}

bool HttpRequest::parseRequestLine(const std::string &line) {
//    std::cout << "parse line : " << line << std::endl;
    // method GET/POST
    size_t pos = line.find("GET");
    if(pos == std::string::npos) {
        pos = line.find("POST");
        if(pos == std::string::npos)
            return false;
        m_method = "POST";
    }else {
        m_method = "GET";
    }
//    std::cout << "method = " << m_method << std::endl;
    // path, example /search?hl=zh-CN&source=hp&q=domety&aq=f
    pos = line.find("/", pos+1);
    if(pos == std::string::npos)
        return false;
    else {
        size_t n_pos = line.find(" ", pos+1);
        if(n_pos == std::string::npos)
            return false;
        m_path = line.substr(pos, n_pos - pos);
//        std::cout << "path = " << m_path << std::endl;
    }
    // HTTP version HTTP/1.1
    pos = line.find("/", pos+1);
    if(pos == std::string::npos)
        return false;
    auto version_s = line.substr(pos + 1);
    if(version_s == "1.0")
        m_version = HTTP_10;
    else if(version_s == "1.1")
        m_version = HTTP_11;
    else
        return false;
    m_state = HEADERS;
//    std::cout << "version = " << version_s << std::endl;
    return true;
}

void HttpRequest::parsePath() {
    if(m_path == "/" || m_path == "/index" || m_path == "/index.htm") {
        m_path = "/index.html";
    }
//    else {
//        for(auto &item: DEFAULT_HTML) {
//            if(item == m_path) {
//                m_path += ".html";
//                break;
//            }
//        }
//    }
}

void HttpRequest::parseHeader(const std::string &line) {
    if(line.empty()) {
        m_state = BODY;
        return;
    }
    size_t pos = line.find(":");
    std::string name = line.substr(0, pos);
    std::string value = line.substr(pos+1);
    m_headers[name] = value;
}

void HttpRequest::parseBody(const std::string &line) {
    //todo
    m_state = FINISH;
}

std::string HttpRequest::path() const {
    return m_path;
}

bool HttpRequest::isKeepAlice() const {
    if(m_headers.count("Connection")) {
        return m_headers.find("Connection")->second == "keep-alive" && m_version == HTTP_11;
    }
    return false;
}
