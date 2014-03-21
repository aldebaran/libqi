#yeah here start all the complication.
#to avoid clash between python2 and python3 we generate each version
#in a separated build folder. (include_directories clash otherwize)

#so we specify the version we want and we build it.

#for python2

if (WITH_PYTHON)
  set(pyver "")
  add_subdirectory(py2py3 "python2")
endif()

if (WITH_PYTHON3)
  #for python3
  set(pyver "3")
  add_subdirectory(py2py3 "python3")
endif()
