### unit tests ###

set(unittest_SRCS
    main.cpp
    telnetparsertest.cpp
)

add_executable(unittest ${unittest_SRCS})
target_link_libraries(unittest gmock gtest Qt5::Test q5250)
target_include_directories(unittest PRIVATE ${CMAKE_SOURCE_DIR}/lib)

add_test(NAME unittest COMMAND unittest)
