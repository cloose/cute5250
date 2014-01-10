include_directories(${CMAKE_SOURCE_DIR}/lib)

macro(create_test NAME)
    add_executable(${NAME} ${NAME}.cpp)
    target_link_libraries(${NAME} q5250)
    qt5_use_modules(${NAME} Test)

    add_test(NAME ${NAME} COMMAND ${NAME})
endmacro(create_test)

### unit tests ###

create_test(generaldatastreamtest)
create_test(telnetcommandstest)
create_test(telnetparsertest)
create_test(terminalemulationtest)

### telnet client unit test ###

set(telnetclienttest_SRCS
    faketelnetserver.cpp
    telnetclienttest.cpp
)

add_executable(telnetclienttest ${telnetclienttest_SRCS})
target_link_libraries(telnetclienttest q5250)
qt5_use_modules(telnetclienttest Network Test)

add_test(NAME telnetclienttest COMMAND telnetclienttest)
