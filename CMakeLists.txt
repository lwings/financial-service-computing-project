cmake_minimum_required(VERSION 3.8)
project(fsc)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        client1/client1.cpp
        client2/client2.cpp
        public/instrument.h
        public/message.h
        server/server.cpp client3/client3.cpp)

add_executable(fsc ${SOURCE_FILES})