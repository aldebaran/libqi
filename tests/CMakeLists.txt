# we still have tests about deprecated stuff, we don't want warnings about that
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")
endif()

add_subdirectory("qi")
add_subdirectory("type")
add_subdirectory("messaging")
