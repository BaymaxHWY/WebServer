//
// Created by heweiyu on 2021/1/30.
//

#ifndef WEBSERVER_HTTPRESPONSE_H
#define WEBSERVER_HTTPRESPONSE_H

#include <string>
#include <unordered_map>
#include <sys/stat.h>
#include <sys/mman.h>

class HttpResponse {
public:
    HttpResponse();
    ~HttpResponse();

    void Init(const std::string &srcDir,const std::string &path, bool isKeepAlive = false, int code = -1);
    void MakeResponse(std::string &buffer);
    void UnmapFile();
    char* File();
    size_t FileSize() const ;
    void ErrorContent(std::string &buffer, const std::string &msg);
    int Code() const {return m_code;}

private:
    void errorHtml();
    void addStateLine(std::string &buffer);
    void addHeaders(std::string &buffer);
    void addContent(std::string &buffer);

    std::string getFileType();

private:
    int m_code;
    bool m_isKeepAlive;

    std::string m_path;
    std::string m_srcDir;

    char *m_mmFile;
    struct stat m_mmFileStat;

    static const std::unordered_map<std::string, std::string> SUFFIX_TYPE;
    static const std::unordered_map<int, std::string> CODE_STATUS;
    static const std::unordered_map<int, std::string> CODE_PATH;
};


#endif //WEBSERVER_HTTPRESPONSE_H
