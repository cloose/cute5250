### unit tests ###

set(unittest_SRCS
    main.cpp
    telnetparsertest.cpp
)

add_executable(unittest ${unittest_SRCS})
target_link_libraries(unittest gmock gtest Qt5::Test)

add_test(NAME unittest COMMAND unittest)
