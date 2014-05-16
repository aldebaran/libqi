cmake_minimum_required(VERSION 2.8)
project(qipython)
find_package(qibuild)

qi_sanitize_compile_flags(HIDDEN_SYMBOLS)
qi_add_optional_package(PYTHON)
qi_add_optional_package(PYTHON3)
# Generate each version in a separated build folder.
# (include_directories clash otherwise)

if (WITH_PYTHON)
  set(pyver "")
  add_subdirectory(py2py3 "python2")
endif()

if (WITH_PYTHON3)
  set(pyver "3")
  add_subdirectory(py2py3 "python3")
endif()
