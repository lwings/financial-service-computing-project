cmake_minimum_required(VERSION 3.8)
project(SmartRouteOrder)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp Order.cpp Order.h OrderBook.cpp OrderBook.h)
add_executable(SmartRouteOrder ${SOURCE_FILES})