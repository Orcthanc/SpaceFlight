cmake_minimum_required( VERSION 3.7 )

set( CMAKE_EXPORT_COMPILE_COMMANDS ON )
set( CMAKE_CXX_STANDARD 20 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )

project( SpaceFlight )

option( COLOR_CONSOLE "Enables colored console output" OFF )

add_subdirectory( external )
add_subdirectory( src )
