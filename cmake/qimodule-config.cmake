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
  # Create a SHARED library instead of MODULE because qibuild does
  # not handle MODULE libraries correctly.
  add_library("${target}" SHARED ${add_library_args})
  target_sources(
    "${target}"
    PRIVATE
      ${qi_create_module_SRC}
  )
  target_link_libraries("${target}" qi::qi)
  set(sdk_dir "${CMAKE_BINARY_DIR}/sdk")
  set(lib_dir "${sdk_dir}/lib/${pkg_subdir}")
  set_target_properties(
    "${target}"
    PROPERTIES
      OUTPUT_NAME "${module_name}"
      LIBRARY_OUTPUT_DIRECTORY "${lib_dir}"
  )


  if(qi_create_module_DEPENDS)
    # Use qibuild macros if available for compatibility.
    if(COMMAND qi_use_lib)
      qi_use_lib("${target}" ${qi_create_module_DEPENDS})
    else()
      message(
        AUTHOR_WARNING
        "Using DEPENDS on `qi_create_module` without importing qibuild "
        "macros (with `find_package(qibuild)`) is not supported anymore, "
        "as `qi_create_module` only exists as a compatibility function "
        "for projects still using `qibuild`.\n"
        "Please use `qi_add_module` and manually link with required "
        "libraries with `target_link_libraries` instead.\n"
        "This function falls back to using `target_link_libraries` "
        "with your `DEPENDS` arguments which might fail if they're not "
        "existing target or library names."
      )
      target_link_libraries("${target}" ${qi_create_module_DEPENDS})
    endif()
  endif()

  set(mod_file_name "${name}.mod")
  set(mod_file_subdir "share/qi/module")
  set(mod_file_build_path "${sdk_dir}/${mod_file_subdir}/${mod_file_name}")
  file(WRITE "${mod_file_build_path}" "cpp\n")

  if(NOT qi_create_module_NO_INSTALL)
    install(
      TARGETS "${target}"
      DESTINATION "lib/${pkg_subdir}"
      COMPONENT runtime
    )
    install(
      FILES "${mod_file_build_path}"
      DESTINATION "${mod_file_subdir}"
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
  set(lib_dir "${sdk_dir}/lib/${pkg_subdir}")
  set_target_properties(
    "${name}"
    PROPERTIES
      OUTPUT_NAME "${module_name}"
      LIBRARY_OUTPUT_DIRECTORY "${lib_dir}"
  )

  set(mod_file_name "${name}.mod")
  set(mod_file_subdir "share/qi/module")
  set(mod_file_build_path "${sdk_dir}/${mod_file_subdir}/${mod_file_name}")
  file(WRITE "${mod_file_build_path}" "cpp\n")

  if(arg_INSTALL)
    install(
      TARGETS "${name}"
      DESTINATION "lib/${pkg_subdir}"
      COMPONENT runtime
    )
    install(
      FILES "${mod_file_build_path}"
      DESTINATION "${mod_file_subdir}"
      COMPONENT runtime
    )
  endif()
endfunction()
