### q5250 library ###

set(q5250_SRCS
    telnetparser.cpp
)

add_definitions(-DQ5250_LIBRARY)

add_library(q5250 SHARED ${q5250_SRCS})
target_link_libraries(q5250 Qt5::Core)
