cmake_minimum_required(VERSION 2.6)
project(ivrworx)

set( CMAKE_VERBOSE_MAKEFILE on )

add_subdirectory(src/kentcsp)
add_subdirectory(src/json_spirit/json_spirit_v4.05)
add_subdirectory(src/iw_core)
