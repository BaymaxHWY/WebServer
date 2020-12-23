cmake_minimum_required(VERSION 3.17)
project(webserver)

set(CMAKE_CXX_STANDARD 17)

add_executable(webserver main.cpp webserver/webserver.h webserver/webserver.cpp threadpool/threadpool.h threadpool/threadpool.cpp http/http_conn.h http/http_conn.cpp util/util.h util/util.cpp)