### q5250 library ###

set(q5250_SRCS
    telnet/telnetclient.cpp
    telnet/telnetparser.cpp
)

add_definitions(-DQ5250_LIBRARY)

add_library(q5250 SHARED ${q5250_SRCS})
qt5_use_modules(q5250 Core)
