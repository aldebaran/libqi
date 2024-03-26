include(${CMAKE_CURRENT_LIST_DIR}/compat-common.cmake)

_qi_set_compat_variables(
  ${CMAKE_FIND_PACKAGE_NAME}
    testsession::qi
    testsession::testsession # testsession requires testssl, so put it first.
    testsession::testssl
    testsession::ka
    testsession::internal::cxx_standard
    testsession::internal::qi_objects
)
