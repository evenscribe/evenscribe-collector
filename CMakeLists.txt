cmake_minimum_required(VERSION 3.5)
project(OlympusObservatory)

set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_COMPILER clang++)

add_executable(${PROJECT_NAME} main.cpp socket.cpp persistence.cpp param.h serializer.cpp)

add_subdirectory(include/clickhouse-cpp)
add_subdirectory(include/json)

target_include_directories(${PROJECT_NAME} PRIVATE include/clickhouse-cpp include/clickhouse-cpp/contrib/absl)
target_link_libraries(${PROJECT_NAME} PRIVATE clickhouse-cpp-lib)

target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json)
