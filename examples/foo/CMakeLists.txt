
cmake_minimum_required(VERSION 2.8)
project(foo-module)
find_package(qibuild)
qi_sanitize_compile_flags(HIDDEN_SYMBOLS)

qi_create_module(foo foo.cpp NO_INSTALL)

qi_create_bin(footest footest.cpp DEPENDS QI NO_INSTALL)
