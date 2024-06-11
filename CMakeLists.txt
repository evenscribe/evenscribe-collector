cmake_minimum_required(VERSION 3.5)
project(OlympusObservatory)

set(CMAKE_CXX_STANDARD 17)

set(SOURCES
    src/main.cpp
    src/socket.cpp
    src/serializer.cpp
    src/cJSON.c
)

add_executable(${PROJECT_NAME} ${SOURCES})

add_subdirectory(include/clickhouse-cpp-2.5.1)
target_link_libraries(${PROJECT_NAME} PRIVATE clickhouse-cpp-lib)

