cmake_minimum_required(VERSION 2.8)
project(input-modifier)

set(CMAKE_CXX_STANDARD 11)

add_executable(imodd
src/daemon/log.cpp
src/daemon/time.cpp
src/daemon/names.c
src/daemon/scanner.cpp
src/daemon/listener.cpp
src/daemon/device.cpp
src/daemon/state.cpp
src/daemon/command.cpp
src/daemon/socket.cpp
src/daemon/main.cpp)

add_executable(imod-uinput-helper
src/helper/log.cpp
src/helper/main.cpp)

add_executable(imod-cli
src/cli/cli.c)

target_link_libraries(imodd pthread)

target_compile_definitions(imodd PUBLIC _PREFIX="${CMAKE_INSTALL_PREFIX}")

install(TARGETS imodd RUNTIME DESTINATION bin)
install(TARGETS imod-uinput-helper RUNTIME DESTINATION bin PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE SETUID)
install(TARGETS imod-cli RUNTIME DESTINATION bin)
