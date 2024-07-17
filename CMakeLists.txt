cmake_minimum_required(VERSION 3.5)
project(OlympusObservatory)

set(CMAKE_CXX_STANDARD 17)

# add_compile_options(-O3 -march=native -mtune=native)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

set(SOURCES
    src/cJSON.c
    src/main.cpp
    src/serializer.cpp
    src/socket.cpp
)

set(HEADERS
    src/cJSON.h
    src/config.h
    src/helper.h
    src/log.h
    src/param.h
    src/persistence.h
    src/query_generator.h
    src/serializer.h
    src/socket.h
)

set(WITH_OPENSSL ON)

add_executable(${PROJECT_NAME} ${SOURCES})

add_subdirectory(include/clickhouse-cpp-2.5.1)
target_link_libraries(${PROJECT_NAME} PRIVATE clickhouse-cpp-lib)

add_subdirectory(include/taopq-1.0.0)
target_link_libraries(${PROJECT_NAME} PRIVATE taopq)
