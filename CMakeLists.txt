cmake_minimum_required(VERSION 2.8)
project(input-modifier)

add_executable(imodd 
src/daemon/log.cpp
src/daemon/scanner.cpp
src/daemon/device.cpp
src/daemon/main.cpp)

target_link_libraries(imodd pthread)
