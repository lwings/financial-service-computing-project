cmake_minimum_required(VERSION 3.8)
project(fsc)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        client1/client1.cpp
        public/Instrument.h
        public/Message.h
<<<<<<< HEAD
        SmartRouteOrder/Order.cpp SmartRouteOrder/OrderBook.cpp AdvOrderBook.cpp AdvOrderBook.h main.cpp)
=======
        SmartRouteOrder/main.cpp SmartRouteOrder/Order.cpp SmartRouteOrder/OrderBook.cpp Exchange/exchange.cpp)
>>>>>>> fda4be159936c90d073a7ff3c817b96bc3d2fd5a

add_executable(fsc ${SOURCE_FILES})