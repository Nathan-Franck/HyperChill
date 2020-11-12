#Synopsis
Playground for modern C++ code, trying out different libraries and toolchains, figuring out the fastest and most useful
processes to write cpp code

#Coding

* Install tools - vcpkg and cmake
* Install dependencies - `vcpkg install glfw3 glad rxcpp glm`
* Make executables - `cmake -DCMAKE_TOOLCHAIN_FILE=[VCPKG location]/scripts/buildsystems/vcpkg.cmake`