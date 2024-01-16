if(NOT TARGET qi::qi)
  include(CMakeFindDependencyMacro)
  find_dependency(qi)
endif()

# Splits the name of a module between the package name and the module name.
# The format of a module name is:
# [<PKG>.]<MODULE>
function(
  _qi_module_name_split
    name
    module_name_var
    pkg_name_var
    pkg_subdir_var
)
  set(module_name ${name})
  set(pkg_name)
  string(FIND ${name} . name_dot_index REVERSE)
  if(name_dot_index GREATER_EQUAL 0)
    # The full name includes a package name with the module name.
    string(SUBSTRING "${name}" 0 ${name_dot_index} pkg_name)
    math(EXPR name_index "${name_dot_index} + 1") # Pass the dot
    string(SUBSTRING "${name}" ${name_index} -1 module_name)
  endif()
  string(REPLACE . / pkg_subdir "${pkg_name}")
  set("${module_name_var}" ${module_name} PARENT_SCOPE)
  set("${pkg_name_var}" ${pkg_name} PARENT_SCOPE)
  set("${pkg_subdir_var}" ${pkg_subdir} PARENT_SCOPE)
endfunction()


# Lookup a dependency and add it to the module target, recursively.
#
# This is a compatibility function to mimic the behavior of `qi_use_lib` for
# `qi_create_module`.
#
# Modules declared with `qi_add_module` should link the module target directly
# to its dependencies with `target_link_libraries`.
function(
  _qi_module_add_dependency
    target
    dependency
)
  # Dependencies names are not case sensitive, try both.
  string(TOLOWER ${dependency} dependency_lower)

  if(TARGET ${dependency})
    target_link_libraries(${target} ${dependency})
  elseif(TARGET ${dependency_lower})
    target_link_libraries(${target} ${dependency_lower})
  else()
    # Not a target, try finding a package with this name.
    string(TOUPPER ${dependency} dependency)
    find_package(${dependency} QUIET REQUIRED)
    if(${dependency}_LIBRARIES)
      target_link_libraries(${target} ${${dependency}_LIBRARIES})
    endif()
    if(${dependency}_INCLUDE_DIRS)
      target_include_directories(${target} PRIVATE ${${dependency}_INCLUDE_DIRS})
    endif()
    if(${dependency}_DEFINITIONS)
      target_compile_definitions(${target} PRIVATE ${${dependency}_DEFINITIONS})
    endif()
    # Recurse into dependencies of the dependency.
    _qi_module_add_dependencies(${target} ${${dependency}_DEPENDS})
  endif()
endfunction()

# Adds each of the arguments as a dependency of the module target.
#
# Consumes but ignores the `ASSUME_SYSTEM_INCLUDE` option that is occasionnally found in
# DEPENDS arguments of `qi_use_lib` or similar functions (including `qi_create_module`).
#
# This is a compatibility function to mimic the behavior of `qi_use_lib` for
# `qi_create_module`.
#
# Modules declared with `qi_add_module` should link the module target directly
# to its dependencies with `target_link_libraries`.
function(
  _qi_module_add_dependencies
    target
)
  cmake_parse_arguments(
    _qi_module_add_dependencies
    # Options
    "ASSUME_SYSTEM_INCLUDE"
    # Single value arguments
    ""
    # Multi value arguments
    ""
    ${ARGN}
  )
  set(dependencies ${_qi_module_add_dependencies_UNPARSED_ARGUMENTS})
  foreach(dependency IN LISTS dependencies)
    _qi_module_add_dependency(${target} ${dependency})
  endforeach()
endfunction()

#[=======================================================================[.rst:
.. command:: qi_create_module

  .. note::

    This command is kept for compatiblity reasons and is considered deprecated.
    Use :command:`qi_add_module` instead.

  Creates a qi module. See :command:`qi_add_module` for more details.

  .. signature::
    qi_create_module(
      [name]
      [EXCLUDE_FROM_ALL]
      [NO_INSTALL]
      <src>...
      [SRC <src>...]
      [DEPENDS <dep>...]
    )
#]=======================================================================]
function(qi_create_module name)
  include(GNUInstallDirs)

  cmake_parse_arguments(
    qi_create_module
    # Options
    "SHARED;NO_INSTALL;EXCLUDE_FROM_ALL"
    # Single value arguments
    ""
    # Multi value arguments
    "SRC;DEPENDS"

    ${ARGN}
  )
  set(add_library_args)
  if(qi_create_module_EXCLUDE_FROM_ALL)
    list(APPEND add_library_args EXCLUDE_FROM_ALL)
  endif()
  list(APPEND qi_create_module_SRC ${qi_create_module_UNPARSED_ARGUMENTS})

  message(STATUS "Module: ${name}")
  _qi_module_name_split("${name}" module_name pkg_name pkg_subdir)

  set(target "${name}")
  add_library("${target}" MODULE ${add_library_args})
  target_sources(
    "${target}"
    PRIVATE
      ${qi_create_module_SRC}
  )
  target_link_libraries("${target}" qi::qi)
  set_target_properties(
    "${target}"
    PROPERTIES
      OUTPUT_NAME "${module_name}"
      LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/sdk/lib/${pkg_subdir}"
  )

  _qi_module_add_dependencies(${target} ${qi_create_module_DEPENDS})

  set(mod_file_name "${name}.mod")
  set(mod_file_subdir "share/qi/module")
  set(mod_file_build_path "${CMAKE_BINARY_DIR}/sdk/${mod_file_subdir}/${mod_file_name}")
  file(WRITE "${mod_file_build_path}" "cpp\n")

  if(NOT qi_create_module_NO_INSTALL)
    install(
      TARGETS "${target}"
      DESTINATION "${CMAKE_INSTALL_LIBDIR}/${pkg_subdir}"
      COMPONENT runtime
    )
    install(
      FILES "${mod_file_build_path}"
      DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/${mod_file_subdir}"
      COMPONENT runtime
    )
  endif()
endfunction()

#[=======================================================================[.rst:
.. command:: qi_add_module

  This command has the same semantics as :command:`add_library` with `MODULE`
  adds a dependency to target `qi::qi`, and takes care of qi module
  files creation and (optionally) installation.

  .. signature::
    qi_add_module(
      [name]
      [INSTALL]
      [<src>...]
    )

  It creates a shared library meant to be loaded dynamically by libqi.

  The name may contain a dot (".") as a separator between the package name and
  the module name. If multiple dots exist in the name, the last one is the
  separator and the previous are considered part of the package name.

  The package name is used as an installation subdirectory for the module. If the
  package name contains dots, they are replaced by a directory separator in the
  destination path.

  Additionally, a "<name>.mod" file describing the module is created and
  installed in a "qi/module" subdirectory of the installation data directory.

  By default, the target is not installed. The `INSTALL` option enables installation
  rules for the module and some additional files.

  Any additional argument is passed as-if to to the call to :command:`add_library`.

  Example
  ^^^^^^^

  .. code-block:: cmake

    qi_add_module(
      naoqi.agility.motion ...
    )

  Creates a module named "motion" for the "naoqi.agility" package.
  On a typical Linux distribution, this will install the following files in
  the installation directory:

  - lib/naoqi/agility/libmotion.so
  - share/qi/module/naoqi.agility.motion.mod
#]=======================================================================]
function(qi_add_module name)
  include(GNUInstallDirs)
  cmake_parse_arguments(
    arg
    # Options
    "INSTALL"
    # Single value arguments
    ""
    # Multi value arguments
    ""
    ${ARGN}
  )
  _qi_module_name_split("${name}" module_name pkg_name pkg_subdir)

  add_library("${name}" MODULE ${arg_UNPARSED_ARGUMENTS})
  target_link_libraries("${name}" PRIVATE qi::qi)
  set(sdk_dir "${CMAKE_BINARY_DIR}/sdk")
  set_target_properties(
    "${name}"
    PROPERTIES
      OUTPUT_NAME "${module_name}"
      LIBRARY_OUTPUT_DIRECTORY "${sdk_dir}/lib/${pkg_subdir}"
  )

  set(mod_file_name "${name}.mod")
  set(mod_file_subdir "share/qi/module")
  set(mod_file_build_path "${sdk_dir}/${mod_file_subdir}/${mod_file_name}")
  file(WRITE "${mod_file_build_path}" "cpp\n")

  if(arg_INSTALL)
    install(
      TARGETS "${name}"
      DESTINATION "${CMAKE_INSTALL_LIBDIR}/${pkg_subdir}"
      COMPONENT runtime
    )
    install(
      FILES "${mod_file_build_path}"
      DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/${mod_file_subdir}"
      COMPONENT runtime
    )
  endif()
endfunction()
