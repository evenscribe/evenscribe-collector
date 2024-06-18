cmake_minimum_required(VERSION 3.20)

PROJECT (VICTORIA-LOGS-INGESTION
    VERSION 0.0.1
    DESCRIPTION "Victoria Logs C++ client for Ingestion"
)

set(CMAKE_CXX_STANDARD 17)
add_subdirectory(src)


# enable_testing()

if(testing_enabled)
    add_subdirectory(tests)
endif()
