cmake_minimum_required(VERSION 2.8)
project(libqiperf)
find_package(qibuild)
enable_testing()

option(WITH_EXAMPLES "Examples" ON)

set(H
  qiperf/dataperfsuite.hpp
  qiperf/dataperf.hpp
  qiperf/api.hpp
)

set(H_P
  src/dataperfsuite_p.hpp
  src/dataperf_p.hpp
)

set(C
  src/dataperfsuite.cpp
  src/dataperf.cpp
)

include_directories(".")
qi_sanitize_compile_flags(HIDDEN_SYMBOLS)

qi_create_lib(qiperf SRC ${H} ${H_P} ${C} SHARED)

qi_use_lib(qiperf QI)

qi_install_header(KEEP_RELATIVE_PATHS ${H})

qi_stage_lib(qiperf)

qi_add_subdirectory(examples WITH_EXAMPLES)
add_subdirectory("tests")

message(STATUS "")
message(STATUS "Build Options:")
message(STATUS "WITH_EXAMPLE : ${WITH_EXAMPLE}")
