# we still have tests about deprecated stuff, we don't want warnings about that
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  include(qi/test_locale.cmake)
endif()

if (UNIX)
endif()

add_subdirectory("qi")
add_subdirectory("type")
add_subdirectory("messaging")
