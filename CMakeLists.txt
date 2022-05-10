cmake_minimum_required(VERSION 3.10)

# HWNS
project(HWNS)

file(GLOB_RECURSE HWNS_HEADERS "${CMAKE_SOURCE_DIR}/HWNS/Source/*.h")
file(GLOB_RECURSE HWNS_SOURCES "${CMAKE_SOURCE_DIR}/HWNS/Source/*.cpp")
add_library(HWNS STATIC "${HWNS_HEADERS}" "${HWNS_SOURCES}")

set_property(TARGET HWNS PROPERTY CXX_STANDARD 20)

target_include_directories(HWNS PUBLIC "${CMAKE_SOURCE_DIR}/HWNS/Source")
target_link_libraries(HWNS PUBLIC ws2_32)

# HWNSClient
project(HWNSClient)

file(GLOB_RECURSE HWNS_CLIENT_HEADERS "${CMAKE_SOURCE_DIR}/HWNSClient/Source/*.h")
file(GLOB_RECURSE HWNS_CLIENT_SOURCES "${CMAKE_SOURCE_DIR}/HWNSClient/Source/*.cpp")
add_executable(HWNSClient "${HWNS_CLIENT_HEADERS}" "${HWNS_CLIENT_SOURCES}")

set_property(TARGET HWNSClient PROPERTY CXX_STANDARD 20)

target_include_directories(HWNSClient PUBLIC "${CMAKE_SOURCE_DIR}/HWNSClient/Source")
target_link_libraries(HWNSClient HWNS)

# HWNSServer
project(HWNSServer)

file(GLOB_RECURSE HWNS_SERVER_HEADERS "${CMAKE_SOURCE_DIR}/HWNSServer/Source/*.h")
file(GLOB_RECURSE HWNS_SERVER_SOURCES "${CMAKE_SOURCE_DIR}/HWNSServer/Source/*.cpp")
add_executable(HWNSServer "${HWNS_SERVER_HEADERS}" "${HWNS_SERVER_SOURCES}")

set_property(TARGET HWNSServer PROPERTY CXX_STANDARD 20)

target_include_directories(HWNSServer PUBLIC "${CMAKE_SOURCE_DIR}/HWNSServer/Source")
target_link_libraries(HWNSServer HWNS)