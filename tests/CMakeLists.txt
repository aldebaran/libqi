add_definitions(-DQI_IS_TEST)

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

add_subdirectory("qi")
add_subdirectory("type")
add_subdirectory("messaging")
add_subdirectory("perf")
add_subdirectory("ka")
