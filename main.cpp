#include <iostream>

#include "WebServer.h"

int main() {
    WebServer server(9999, 8, 60000); // 超时 60s
    server.start();
    return 0;
}

/*
 * webbench
 *  webbench -c 100 -t 10 http://118.193.37.13:9999/
 * */