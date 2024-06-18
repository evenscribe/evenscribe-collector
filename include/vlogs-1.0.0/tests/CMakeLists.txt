set(TEST_SOURCES
    test_client.cpp
)


include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(VictoriaLogsTests ${TEST_SOURCES})

target_link_libraries(VictoriaLogsTests PRIVATE victorialogslib)

add_test(NAME VictoriaLogsTests COMMAND VictoriaLogsTests)
