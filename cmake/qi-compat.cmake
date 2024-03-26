include(${CMAKE_CURRENT_LIST_DIR}/compat-common.cmake)

_qi_set_compat_variables(
  ${CMAKE_FIND_PACKAGE_NAME}
    qi::qi
    qi::ka
    qi::internal::cxx_standard
    qi::internal::qi_objects
)
