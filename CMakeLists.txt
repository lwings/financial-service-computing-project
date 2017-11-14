cmake_minimum_required(VERSION 3.8)
project(fsc)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        client1/client1.cpp
        public/Instrument.h
        public/Message.h
        SmartRouteOrder/main.cpp SmartRouteOrder/Order.cpp SmartRouteOrder/OrderBook.cpp)

add_executable(fsc ${SOURCE_FILES})