cmake_minimum_required(VERSION 3.5)
project(projekt_so)

find_package( Curses REQUIRED )
include_directories( ${CURSES_INCLUDE_DIRS} )
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp)
add_executable(projekt_so ${SOURCE_FILES})
target_link_libraries( projekt_so ${CURSES_LIBRARIES} )
target_link_libraries( projekt_so pthread )


