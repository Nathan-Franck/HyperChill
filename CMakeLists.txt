cmake_minimum_required(VERSION 3.0)
project(HyperChill)

set(CMAKE_CXX_STANDARD 20)
# set(DCMAKE_TOOLCHAIN_FILE "./vcpkg.cmake")

find_package(glad CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(rxcpp CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)


add_executable(HyperChillGame "src/main.cpp")

target_link_libraries(HyperChillGame glad::glad)
target_link_libraries(HyperChillGame OpenGL::GL)
target_link_libraries(HyperChillGame glfw)
target_link_libraries(HyperChillGame glm)

# add_executable(ReactiveTester "src/reactive_tester.cpp")
# target_link_libraries(ReactiveTester glad::glad)
# target_link_libraries(ReactiveTester OpenGL::GL)
# target_link_libraries(ReactiveTester glfw)
# target_link_libraries(ReactiveTester rxcpp)
# target_link_libraries(ReactiveTester glm)
