### unit tests ###

set(unittest_SRCS
    main.cpp
    generaldatastreamtest.cpp
    telnetclienttest.cpp
    telnetparsertest.cpp
)

add_executable(unittest ${unittest_SRCS})
target_link_libraries(unittest pthread gmock gtest q5250)
target_include_directories(unittest PRIVATE ${CMAKE_SOURCE_DIR}/lib)
qt5_use_modules(unittest Core Test)

add_test(NAME unittest COMMAND unittest)
