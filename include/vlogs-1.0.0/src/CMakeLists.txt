cmake_minimum_required(VERSION 3.20)
set(SOURCES
    client_safe.cpp
    client.cpp
)

include(FetchContent)
include(GenerateExportHeader)
include(CMakePackageConfigHelpers)

FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/libcpr/cpr.git
                         GIT_TAG 871ed52d350214a034f6ef8a3b8f51c5ce1bd400)
FetchContent_MakeAvailable(cpr)


add_library(victorialogslib ${SOURCES})
set_target_properties (victorialogslib
    PROPERTIES
    LINKER_LANGUAGE CXX
    VERSION 0.1.1
)

target_link_libraries(victorialogslib PUBLIC cpr::cpr)
target_include_directories(victorialogslib PUBLIC ${PROJECT_SOURCE_DIR}/include)


INSTALL (TARGETS victorialogslib
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

INSTALL(DIRECTORY ${CMAKE_SOURCE_DIR}/include/victorialogslib
        DESTINATION include)
