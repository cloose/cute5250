include_directories(${CMAKE_SOURCE_DIR}/lib)

### cute5250 application ###

set(cute5250_SRCS
    main.cpp
    mainwindow.cpp
)

qt5_wrap_ui(cute5250_SRCS mainwindow.ui)

add_executable(cute5250 ${cute5250_SRCS})
target_link_libraries(cute5250 q5250)
qt5_use_modules(cute5250 Widgets)
