//
// Created by heweiyu on 2021/1/31.
//

#ifndef WEBSERVER_NONCOPYABLE_H
#define WEBSERVER_NONCOPYABLE_H


class noncopyable {
protected:
    noncopyable() = default;
    ~noncopyable() = default;

public:
    noncopyable(const noncopyable&) = delete;
    const noncopyable& operator= (const noncopyable&) = delete;
};

#endif //WEBSERVER_NONCOPYABLE_H
