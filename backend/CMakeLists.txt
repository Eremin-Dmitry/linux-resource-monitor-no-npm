cmake_minimum_required(VERSION 3.16)
project(linux_resource_monitor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)

add_executable(resource-monitor
    src/ResourceMonitorApplication.cpp
    src/SystemResourceMonitor.cpp
    src/ProcessResourceMonitor.cpp
    src/ResourceWebSocketServer.cpp
)

target_include_directories(resource-monitor PRIVATE src)
target_link_libraries(resource-monitor PRIVATE OpenSSL::Crypto Threads::Threads)

target_compile_options(resource-monitor PRIVATE -Wall -Wextra -Wpedantic)
