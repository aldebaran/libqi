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
