# we still have tests about deprecated stuff, we don't want warnings about that
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")
endif()

if(QI_WITH_TESTS)
  include_directories(".")
  qi_create_lib(qitestutils
    STATIC NO_INSTALL
    SRC
      "qi/testutils/testutils.hpp"
      "qi/testutils/testutils.cpp"
      "qi/testutils/mockutils.hpp"
    DEPENDS qi gmock
  )
  qi_stage_lib(qitestutils)
endif()

add_subdirectory("qi")
add_subdirectory("type")
add_subdirectory("messaging")
add_subdirectory("perf")
