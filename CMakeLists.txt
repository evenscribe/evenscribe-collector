cmake_minimum_required(VERSION 3.5)
project(OlympusObservatory)

set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_COMPILER clang++)

add_subdirectory(deps/clickhouse-cpp)

add_executable(${PROJECT_NAME} main.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE deps/clickhouse-cpp deps/clickhouse-cpp/contrib/absl)

target_link_libraries(${PROJECT_NAME} PRIVATE clickhouse-cpp-lib)
