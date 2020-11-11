cmake_minimum_required(VERSION 3.17)
project(HyperChill)

set(CMAKE_CXX_STANDARD 17)

add_executable(HyperChillGame "src/main.cpp")
add_executable(HyperChillEditor "src/editor.cpp")
add_executable(ReactiveTester "src/reactive_tester.cpp")

# set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
# set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)

add_subdirectory("deps/glfw")
add_subdirectory("deps/glad")
add_subdirectory("deps/RxCpp")

target_link_libraries(HyperChillGame glfw)
target_link_libraries(HyperChillGame glad)
target_link_libraries(HyperChillEditor glfw)
target_link_libraries(HyperChillEditor glad)
target_link_libraries(HyperChillEditor glfw)
target_link_libraries(ReactiveTester glad)
target_link_libraries(ReactiveTester glfw)
target_link_libraries(ReactiveTester rxcpp)

find_package(OpenGL REQUIRED)
target_link_libraries(HyperChillGame OpenGL::GL)
find_package(OpenGL REQUIRED)
target_link_libraries(HyperChillEditor OpenGL::GL)
