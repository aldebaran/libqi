include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../)

set(_headers
  qipython/api.hpp
  qipython/pyinit.hpp
  qipython/pyexport.hpp
  qipython/pyapplication.hpp
  qipython/pysession.hpp
  qipython/pyfuture.hpp
  qipython/pyobject.hpp
  qipython/pyproperty.hpp
  qipython/pythreadsafeobject.hpp
  qipython/pysignal.hpp
  qipython/pyasync.hpp
  qipython/pyobjectfactory.hpp
  qipython/pylog.hpp
  qipython/pypath.hpp
  qipython/pytranslator.hpp
  qipython/error.hpp
  qipython/gil.hpp
)

set(_sources
  src/pyinit.cpp
  src/pyexport.cpp
  src/pyapplication.cpp
  src/pysession.cpp
  src/pyfuture.cpp
  src/pyobject.cpp
  src/pyobjectconverter.cpp
  src/pyproperty.cpp
  src/pysignal.cpp
  src/pyasync.cpp
  src/pyobjectfactory.cpp
  src/pylog.cpp
  src/pypath.cpp
  src/pytranslator.cpp
)

qi_create_lib(qipython SHARED
  ${_headers}
  ${_sources}
)

qi_use_lib(qipython BOOST_PYTHON QI QITYPE QIMESSAGING PYTHON)

qi_install_header(${_headers} KEEP_RELATIVE_PATHS)

qi_stage_lib(qipython)

# do not use qi_create_lib: it does not all
# the python-specific stuff we need.
# FIXME: use qi_create_py_ext() from qibuild
# when it is implemented
add_library(_qi SHARED
  src/boostpymain.cpp
)

# Fix the library name: no lib prefix, correct extension
set_target_properties(_qi PROPERTIES PREFIX "")
if(UNIX)
  set_target_properties(_qi PROPERTIES
    SUFFIX ".so"
    LIBRARY_OUTPUT_DIRECTORY ${QI_SDK_DIR}/${QI_SDK_LIB}
  )
endif()

#set rpath so that _qi can find its dependencies when loaded.
if(UNIX AND NOT APPLE)
  # Use a relative rpath at installation
  set_target_properties(_qi
    PROPERTIES
      INSTALL_RPATH "\$ORIGIN/../lib"
  )
endif()

if (WIN32)
  set_target_properties(_qi PROPERTIES
    SUFFIX ".pyd"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${QI_SDK_DIR}/${QI_SDK_LIB}
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${QI_SDK_DIR}/${QI_SDK_LIB}
  )
endif()

qi_use_lib(_qi QIPYTHON)

# Install the library next to the .py files
qi_install_python(TARGETS _qi)

qi_install_python(${CMAKE_CURRENT_SOURCE_DIR}/qi
  KEEP_RELATIVE_PATHS
)

# Add automatic testing for Python bindings
add_subdirectory(test)
