
cmake_minimum_required(VERSION 3.20)
project(IMDB_Query C)

find_package(CURL REQUIRED)
find_package(cjson CONFIG REQUIRED)

set(RESOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/resource")

configure_file(
        "${RESOURCE_DIR}/title.txt"
        "${CMAKE_CURRENT_BINARY_DIR}/title.txt"
        COPYONLY
)

add_executable(IMDB_Query
        main.c
        util.c util.h
        IQuery.c IQuery.h
        entities.c entities.h
        binService.c binService.h
        dbContext.c dbContext.h
        apiHandler.c apiHandler.h
        sysManager.c sysManager.h)

target_link_libraries(IMDB_Query PRIVATE
        CURL::libcurl
        cjson)

target_compile_definitions(IMDB_Query PRIVATE
        TITLE_FILE_PATH="${CMAKE_CURRENT_BINARY_DIR}/title.txt"
)