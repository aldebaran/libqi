cmake_minimum_required(VERSION 3.16) # Ubuntu 20.04 comes with CMake 3.16.3
project(foo-module)
find_package(qibuild)
qi_sanitize_compile_flags(HIDDEN_SYMBOLS)

qi_create_module(foo foo.cpp NO_INSTALL)

qi_create_bin(footest footest.cpp DEPENDS QI NO_INSTALL)
