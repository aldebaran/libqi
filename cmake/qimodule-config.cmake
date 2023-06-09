if(NOT TARGET qi::qi)
  include(CMakeFindDependencyMacro)
  find_dependency(qi)
endif()

#[=======================================================================[.rst:
Creates a qi module.

.. code-block:: cmake

  qi_create_module(
    [name]
    [EXCLUDE_FROM_ALL]
    [NO_INSTALL]
    <src>...
    [SRC <src>...]
    [DEPENDS <dep>...]
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

Example
^^^^^^^

.. code-block:: cmake

  qi_create_module(
    naoqi.agility.motion ...
  )

Creates a module named "motion" for the "naoqi.agility" package.
On a typical Linux distribution, this will install the following files in
the installation directory:

- lib/naoqi/agility/libmotion.so
- share/qi/module/naoqi.agility.motion.mod
#]=======================================================================]
function(qi_create_module name)
  include(GNUInstallDirs)

  cmake_parse_arguments(
    qi_create_module
    # Options
    "NO_INSTALL;EXCLUDE_FROM_ALL"
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

  set(name_with_pkg "${name}")
  message(STATUS "Module: ${name_with_pkg}")

  string(FIND "${name_with_pkg}" . name_dot_index REVERSE)

  # The full name is only the module name.
  if(name_dot_index EQUAL -1)
    set(pkg_name)
  # The full name includes a package name with the module name.
  else()
    string(SUBSTRING "${name_with_pkg}" 0 "${name_dot_index}" pkg_name)
    math(EXPR name_index "${name_dot_index} + 1") # Pass the dot
    string(SUBSTRING "${name_with_pkg}" "${name_index}" -1 name)
  endif()

  string(REPLACE . / pkg_subdir "${pkg_name}")

  set(target "${name_with_pkg}")
  add_library("${target}" MODULE ${add_library_args})
  target_sources(
    "${target}"
    PRIVATE
      ${qi_create_module_SRC}
  )
  target_link_libraries(
    "${target}"
    PRIVATE
      qi::qi
  )
  set_target_properties(
    "${target}"
    PROPERTIES
      OUTPUT_NAME "${name}"
      LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/sdk/lib/${pkg_subdir}"
  )

  foreach(dep IN LISTS qi_create_module_DEPENDS)
    # Dependencies names are not case sensitive, try both.
    string(TOLOWER "${dep}" ldep)
    if(TARGET "${dep}")
      target_link_libraries("${target}" PRIVATE "${dep}")
    elseif(TARGET "${ldep}")
      target_link_libraries("${target}" PRIVATE "${ldep}")
    else()
      # Not a target, try finding a package with this name.
      string(TOUPPER "${dep}" dep)
      find_package("${dep}")
      if("${dep}_FOUND")
        if("${dep}_LIBRARIES")
          target_link_libraries("${target}" PRIVATE ${${dep}_LIBRARIES})
        endif()
        if("${dep}_INCLUDE_DIRS")
          target_include_directories(${target} PRIVATE ${${dep}_INCLUDE_DIRS})
        endif()
        if("${dep}_DEFINITIONS")
          target_compile_definitions(${target} PRIVATE ${${dep}_DEFINITIONS})
        endif()
      endif()
    endif()
  endforeach()

  set(mod_file_name "${name_with_pkg}.mod")
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
    )
  endif()
endfunction()
