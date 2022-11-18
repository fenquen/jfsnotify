cmake_minimum_required(VERSION 3.23)
project(jfsnotify_c)

set(CMAKE_CXX_STANDARD 11)

set(JAVA_HOME "/home/a/program/azul")
include_directories(${JAVA_HOME}/include ${JAVA_HOME}/include/linux)

link_directories(${JAVA_HOME}/jre/lib/amd64)

add_library(jfsnotify SHARED fsnotify.cpp common.cpp)
