### q5250 library ###

set(q5250_SRCS
    generaldatastream.cpp
    telnet/tcpsockettelnetconnection.cpp
    telnet/telnetclient.cpp
    telnet/telnetparser.cpp
    terminal/cursor.cpp
    terminal/field.cpp
    terminal/terminaldisplaybuffer.cpp
    terminal/terminalemulator.cpp
    terminal/terminalformattable.cpp
)

add_definitions(-DQ5250_LIBRARY)

add_library(q5250 SHARED ${q5250_SRCS})
qt5_use_modules(q5250 Core Network)

if (BUILD_WITH_CODE_COVERAGE)
  add_definitions (--coverage)
  target_link_libraries(q5250 gcov)
endif()
