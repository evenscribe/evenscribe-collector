cmake_minimum_required(VERSION 3.5)
project(OlympusObservatory)

set(CMAKE_CXX_STANDARD 17)

# Include all source files in the src/ directory and its subdirectories
file(GLOB_RECURSE SOURCE_FILES "src/*.cpp")

# Include all header files in the src/ directory and its subdirectories
file(GLOB_RECURSE HEADER_FILES "src/*.h")

add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES})

add_subdirectory(include/clickhouse-cpp-2.5.1)
target_link_libraries(${PROJECT_NAME} PRIVATE clickhouse-cpp-lib)

