cmake_minimum_required(VERSION 3.21)

project(server LANGUAGES CXX)
project(client LANGUAGES CXX)

# Set target directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/)

# Include SourceFiles.cmake to access the SOURCEFILES and INCLUDEDIRS variables
include(${CMAKE_SOURCE_DIR}/src/SourceFiles.cmake)

add_executable(server ${SERVERSOURCEFILES})
add_executable(client ${CLIENTSOURCEFILES})

target_link_libraries(server PRIVATE zmq)
target_link_libraries(client PRIVATE zmq)

target_include_directories(server PUBLIC ${INCLUDEDIRS} ${CMAKE_SOURCE_DIR}/deps/cppzmq-4.10.0)
target_include_directories(client PUBLIC ${INCLUDEDIRS} ${CMAKE_SOURCE_DIR}/deps/cppzmq-4.10.0)