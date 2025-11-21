
cmake_minimum_required(VERSION 3.20)
project(IMDB-Query C)

find_package(CURL REQUIRED)
find_package(cjson CONFIG REQUIRED)

add_executable(IMDB_Query main.c
        util.c
        util.h
        IQuery.c
        IQuery.h)


target_link_libraries(IMDB_Query PRIVATE
        CURL::libcurl
        cjson

)