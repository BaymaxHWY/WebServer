//
// Created by heweiyu on 2021/1/25.
//

#ifndef WEBSERVER_HTTPREQUEST_H
#define WEBSERVER_HTTPREQUEST_H

#include <string>
#include <unordered_map>
#include <unordered_set>

class HttpRequest {
public:
    enum PARSE_STATE{
        REQUEST_LINE, //    请求行 GET /index.html HTTP/1.1
        HEADERS, // 请求头部
        BODY, // 请求数据
        FINISH, // 结束
    };
    enum HTTP_CODE{
        NO_REQUEST = 0, // 请求不完整需要继续读
        GET_REQUEST, // 表示一个完整的客户请求
        BAD_REQUEST, // 存在语法错误
        NO_RESOURSE, // 资源不存在
        FORBIDDENT_REQUEST, // 客户对资源没有访问权限
        FILE_REQUEST, // 文件资源
        INTERNAL_ERROR, // 服务器内部错误
        CLOSED_CONNECTION, // 客户端已经关闭连接
    };
    enum HTTP_VERSION{
        HTTP_10,
        HTTP_11,
    };

    HttpRequest() {init();}
    ~HttpRequest() = default;

    void init();
    bool parse(std::string& buffer);

    bool isKeepAlice() const;

    std::string path() const;

private:
    bool parseRequestLine(const std::string&);
    void parsePath();
    void parseHeader(const std::string&);
    void parseBody(const std::string&);
private:
    PARSE_STATE m_state;
    std::string m_method, m_path;
    HTTP_VERSION m_version;
    std::unordered_map<std::string, std::string> m_headers;
    std::unordered_map<std::string, std::string> m_posts;

//    static const std::unordered_set<std::string> DEFAULT_HTML;
//    static const std::unordered_map<std::string, int> DEFAULT_HTML_TAG;
};


#endif //WEBSERVER_HTTPREQUEST_H
