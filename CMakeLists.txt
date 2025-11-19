cmake_minimum_required(VERSION 3.31.6)
project(IMDB_Query C)

set(CMAKE_C_STANDARD 11)

add_executable(IMDB_Query main.c systemManager.c entities.c)
