include_directories(${CMAKE_SOURCE_DIR}/lib)

### generaldatastream unit test ###

set(generaldatastreamtest_SRCS
    generaldatastreamtest.cpp
)

add_executable(generaldatastreamtest ${generaldatastreamtest_SRCS})
target_link_libraries(generaldatastreamtest q5250)
qt5_use_modules(generaldatastreamtest Test)

add_test(NAME generaldatastreamtest COMMAND generaldatastreamtest)

### telnet client unit test ###

set(telnetclienttest_SRCS
    faketelnetserver.cpp
    telnetclienttest.cpp
)

add_executable(telnetclienttest ${telnetclienttest_SRCS})
target_link_libraries(telnetclienttest q5250)
qt5_use_modules(telnetclienttest Network Test)

add_test(NAME telnetclienttest COMMAND telnetclienttest)
