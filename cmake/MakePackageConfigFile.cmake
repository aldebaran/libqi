function(make_package_config_file export pkg_name)
  cmake_parse_arguments(mpcf "" COMPONENT "" ${ARGN})

  string(TOUPPER "${pkg_name}" upper_pkg_name)

  set(component)
  if(mpcf_COMPONENT)
    set(component COMPONENT ${mpcf_COMPONENT})
  endif()

  # Export inside the build tree.
  export(
    EXPORT "${export}"
    NAMESPACE "${pkg_name}::"
    FILE "${pkg_name}-targets.cmake"
  )

  # Export inside the install tree.
  set(
    "${upper_pkg_name}_INSTALL_CMAKEDIR"
    "${CMAKE_INSTALL_LIBDIR}/cmake/${pkg_name}"
    CACHE STRING
    "Path where ${pkg_name} package CMake configuration files are to be installed."
  )

  install(
    EXPORT "${export}"
    DESTINATION "${${upper_pkg_name}_INSTALL_CMAKEDIR}"
    NAMESPACE "${pkg_name}::"
    FILE "${pkg_name}-targets.cmake"
    ${component}
  )

  set(
    config_file_path
    "${CMAKE_CURRENT_BINARY_DIR}/${pkg_name}-config.cmake"
  )
  set(PACKAGE_NAME "${pkg_name}")
  configure_package_config_file(
    cmake/config.cmake.in
    "${config_file_path}"
    INSTALL_DESTINATION "${${upper_pkg_name}_INSTALL_CMAKEDIR}"
  )

  set(
    config_version_file_path
    "${CMAKE_CURRENT_BINARY_DIR}/${pkg_name}-config-version.cmake"
  )
  write_basic_package_version_file(
    "${config_version_file_path}"
    COMPATIBILITY SameMajorVersion # SemVer
  )

  install(
    FILES
      "${config_file_path}"
      "${config_version_file_path}"
    DESTINATION "${${upper_pkg_name}_INSTALL_CMAKEDIR}"
    ${component}
  )
endfunction()
