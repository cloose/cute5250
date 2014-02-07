### integration tests ###

set(integrationtest_SRCS
    main.cpp
    tcpsockettelnetconnectiontest.cpp
)

add_executable(integrationtest ${integrationtest_SRCS})
target_link_libraries(integrationtest pthread gmock gtest q5250)
target_include_directories(integrationtest PRIVATE ${CMAKE_SOURCE_DIR}/lib)
qt5_use_modules(integrationtest Core Network Test)

add_test(NAME integrationtest COMMAND integrationtest)
