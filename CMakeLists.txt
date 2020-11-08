cmake_minimum_required(VERSION 3.17)
project(HyperChill)

set(CMAKE_CXX_STANDARD 14)

add_executable(HyperChill main.cpp)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(glfw)
target_link_libraries(HyperChill glfw)
add_subdirectory(glad)
target_link_libraries(HyperChill glad)

find_package(OpenGL REQUIRED)
target_link_libraries(HyperChill OpenGL::GL)
