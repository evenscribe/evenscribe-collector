cmake_minimum_required(VERSION 3.5)
project(evenscribe)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

file(GLOB_RECURSE SOURCE_FILES "src/*.cpp" "src/*.c")
file(GLOB_RECURSE HEADER_FILES "src/*.h")

add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES})

add_subdirectory(include/clickhouse-cpp-2.5.1)
target_link_libraries(${PROJECT_NAME} PRIVATE clickhouse-cpp-lib)

add_subdirectory(include/taopq-1.0.0)
target_link_libraries(${PROJECT_NAME} PRIVATE taopq)
