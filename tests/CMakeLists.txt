if(NOT BUILD_TESTING)
    return()
endif()

find_package(GTest REQUIRED)

##############################################################################
# Library: testutils
##############################################################################
add_library(
    testutils
    STATIC
    EXCLUDE_FROM_ALL
)
add_library(qi::testutils ALIAS testutils)

target_sources(
  testutils
  PUBLIC FILE_SET HEADERS
    FILES
      qi/testutils/testutils.hpp
      qi/testutils/mockutils.hpp
  PRIVATE
    qi/testutils/testutils.cpp
)

target_include_directories(
  testutils
  PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}"
)

target_link_libraries(
  testutils
  PUBLIC
    qi::internal::cxx_standard
    qi::qi
    GTest::gtest
    GTest::gmock
)

# Compatibility functions for legacy qibuild macros.
function(qi_create_test_helper name)
  cmake_parse_arguments(qcth "" "" "DEPENDS" ${ARGN})
  add_executable("${name}")
  target_sources("${name}" PRIVATE ${qcth_UNPARSED_ARGUMENTS})
  target_link_libraries("${name}" PRIVATE ${qcth_DEPENDS})
endfunction()

function(qi_create_test name)
  cmake_parse_arguments(qct "" "TIMEOUT" "SRC;ARGUMENTS;DEPENDS" ${ARGN})
  add_executable("${name}")
  target_sources("${name}" PRIVATE ${qct_SRC} ${qct_UNPARSED_ARGUMENTS})
  target_link_libraries("${name}" PRIVATE qi::internal::cxx_standard GTest::gtest)
  if(qct_DEPENDS)
    target_link_libraries("${name}" PRIVATE ${qct_DEPENDS})
  endif()
  set(args)
  if(qct_ARGUMENTS)
    set(args ${qct_ARGUMENTS})
  endif()
  add_test(NAME "${name}" COMMAND "${name}" ${args})
  if(qct_TIMEOUT)
    set_target_properties(
      "${name}"
      PROPERTIES TIMEOUT "${qct_TIMEOUT}"
    )
  endif()
endfunction()

add_subdirectory(qi)
add_subdirectory(type)
add_subdirectory(messaging)
add_subdirectory(perf)
add_subdirectory(ka)
