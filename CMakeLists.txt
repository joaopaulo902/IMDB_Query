cmake_minimum_required(VERSION 3.31.6)
project(IMDB-Query C)

set(CMAKE_C_STANDARD 11)

find_package(CURL REQUIRED)
add_executable(IMDB_Query main.c
        util.c
        util.h)

include(FetchContent)

FetchContent_Declare(
        cjson
        GIT_REPOSITORY https://github.com/DaveGamble/cJSON.git
        GIT_TAG        v1.7.18
)

set(ENABLE_CJSON_TEST OFF CACHE BOOL "cJSON: Build with unit testing" FORCE)

FetchContent_MakeAvailable(cjson)

if (MINGW)
    target_compile_options(cjson PRIVATE -Wno-error=float-conversion)
endif()

target_link_libraries(IMDB_Query PRIVATE CURL::libcurl
cjson)