cmake_minimum_required(VERSION 3.5)
project(OlympusObservatory)

set(CMAKE_CXX_STANDARD 17)

set(SOURCES
    src/main.cpp
    src/socket.cpp
    src/query_generator.cpp
    src/serializer.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

add_subdirectory(include/clickhouse-cpp-2.5.1)
target_link_libraries(${PROJECT_NAME} PRIVATE clickhouse-cpp-lib)

add_subdirectory(include/json-3.11.3)
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)
