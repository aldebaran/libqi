include(${CMAKE_CURRENT_LIST_DIR}/compat-common.cmake)

set_compat_variables(
  ${CMAKE_FIND_PACKAGE_NAME}
    testsession::qi
    testsession::testssl
    testsession::testsession
    testsession::ka
    testsession::internal::cxx_standard
    testsession::internal::qi_objects
)
