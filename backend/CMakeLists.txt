cmake_minimum_required(VERSION 3.16)
project(linux_resource_monitor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Version information
set(PROJECT_VERSION_MAJOR 1)
set(PROJECT_VERSION_MINOR 0)
set(PROJECT_VERSION_PATCH 0)
set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)

# Main executable
add_executable(resource-monitor
    src/ResourceMonitorApplication.cpp
    src/SystemResourceMonitor.cpp
    src/ProcessResourceMonitor.cpp
    src/ResourceWebSocketServer.cpp
    src/Config.cpp
    src/Logger.cpp
)

target_include_directories(resource-monitor PRIVATE src)
target_link_libraries(resource-monitor PRIVATE OpenSSL::Crypto Threads::Threads)

# Compiler flags and options
target_compile_options(resource-monitor PRIVATE -Wall -Wextra -Wpedantic)

# Set version information
set_target_properties(resource-monitor PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

# Enable position-independent code
set_target_properties(resource-monitor PROPERTIES POSITION_INDEPENDENT_CODE ON)

# Installation rules
install(TARGETS resource-monitor DESTINATION bin)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/../.env.example DESTINATION etc/resource-monitor)
