# the minimum version of CMake required
cmake_minimum_required(VERSION 3.6.2)

set (CMAKE_CXX_COMPILER "clang++" )
set (CMAKE_C_COMPILER "clang" )
set (CMAKE_CXX_STANDARD 14)

project(glstuto)
add_executable(glstuto main.cpp)
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package(GLEW REQUIRED)

include_directories( ${OPENGL_INCLUDE_DIRS}  ${GLUT_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS} )

target_link_libraries(glstuto ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} ${GLEW_LIBRARIES} )
target_link_libraries(glstuto chemfiles)
