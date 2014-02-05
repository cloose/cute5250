### q5250 library ###

set(q5250_SRCS
    telnet/telnetclient.cpp
    telnet/telnetparser.cpp
)

add_definitions(-DQ5250_LIBRARY)

add_library(q5250 SHARED ${q5250_SRCS})
qt5_use_modules(q5250 Core)

if (BUILD_WITH_CODE_COVERAGE)
  add_definitions (--coverage)
  target_link_libraries(q5250 gcov)
endif()
