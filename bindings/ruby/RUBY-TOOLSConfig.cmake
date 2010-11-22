###
#
# Could one day contain stuff such as:
#
#   - Wrappers around SWIG
#
#   - Auto-generation of setup.py files
#
#   - Adding a "python setup.py install --prefix=CMAKE_INSTALL_PREFIX" command
#
#  ....

include("${TOOLCHAIN_DIR}/cmake/libfind.cmake")


#############
#
# Nice wrapper for swig.
#
# wrap_python(module_name interface_file SRCS srcs... DEPENDENCIES deps ...)
#
# /!\ The module_name must be the same as the one declare in ${interface_file}
# for instance, if module_name equals foo, foo.i must contain:
#   %module foo
##############
function(wrap_ruby module_name interface_file)

  ##
  # Parse args:
  subargs_parse_args("SRCS" "DEPENDENCIES" _srcs _deps ${ARGN})
  if (_deps)
    subargs_parse_args("DEPENDENCIES" "" _deps _tmp ${_deps})
  endif()


  ##
  # Basic configurations
  find_package(SWIG REQUIRED)
  include(${SWIG_USE_FILE})
  set_source_files_properties(${interface_file} PROPERTIES CPLUSPLUS ON)
  # tell swig that the generated module name is ${module_name}.py
  # without this property, it assumes that it is ${interface_file}.py
  # TODO: check that it is a correct way to do this and not a nifty hack
  # set_source_files_properties(
  #   ${interface_file} PROPERTIES SWIG_MODULE_NAME "${module_name}")

  # Everything will end up in ${SDK_DIR}/${SDK_DIR}, so that
  # setting PYTHONPATH and LD_LIBRARY_PATH (or PATH) is enough
  set(CMAKE_SWIG_OUTDIR ${SDK_DIR}/${_SDK_LIB})

  ##
  # Deal with dependencies:
  foreach (_dep ${_deps})
    find(${_dep})
    include_directories(${${_dep}_INCLUDE_DIR})
  endforeach()

  # Since there is often a "lazy" include in the interface file:
  include_directories(${CMAKE_CURRENT_SOURCE_DIR})

  swig_add_module(${module_name} ruby ${interface_file} ${_srcs})

  ##
  # Deal with the newly created target

  # Store the target created by swig_add_module in a more friendly name:
  set(_swig_target ${SWIG_MODULE_${module_name}_REAL_NAME})

  #TODO
  use_lib(${_swig_target} RUBY ${_deps})

  set_target_properties(${_swig_target}
    PROPERTIES
      LIBRARY_OUTPUT_DIRECTORY "${SDK_DIR}/${_SDK_LIB}"
  )

  #remove the prefix 'lib' from the library name
  set_target_properties(${_swig_target}
    PROPERTIES
      PREFIX "")

  # Re-create install rules:
  install(TARGETS ${_swig_target}
    COMPONENT python
    LIBRARY DESTINATION "${_SDK_LIB}"
    RUNTIME DESTINATION "${_SDK_LIB}"
  )

  # install(FILES "${SDK_DIR}/${_SDK_LIB}/${module_name}.py"
  #   COMPONENT ruby
  #   DESTINATION "${_SDK_LIB}"
  #)


endfunction()
