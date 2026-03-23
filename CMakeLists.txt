cmake_minimum_required(VERSION 3.28)

project(ExchangeSimulator CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(ExchangeSimulator src/main.cpp "src/Order.ixx")

target_sources(ExchangeSimulator
    PUBLIC
        FILE_SET CXX_MODULES FILES
        src/Order.ixx
)