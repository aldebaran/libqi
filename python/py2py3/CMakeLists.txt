
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

set(_headers
  ../qipython/api.hpp
  ../qipython/pyinit.hpp
  ../qipython/pyexport.hpp
  ../qipython/pyapplication.hpp
  ../qipython/pysession.hpp
  ../qipython/pyfuture.hpp
  ../qipython/pyobject.hpp
  ../qipython/pyproperty.hpp
  ../qipython/pythreadsafeobject.hpp
  ../qipython/pysignal.hpp
  ../qipython/pyasync.hpp
  ../qipython/pyobjectfactory.hpp
  ../qipython/pylog.hpp
  ../qipython/pypath.hpp
  ../qipython/pytranslator.hpp
  ../qipython/error.hpp
  ../qipython/gil.hpp
  ../qipython/trace.hpp
)

set(_sources
  ../src/pyinit.cpp
  ../src/pyexport.cpp
  ../src/pyapplication.cpp
  ../src/pysession.cpp
  ../src/pyfuture.cpp
  ../src/pyobject.cpp
  ../src/pyobjectconverter.cpp
  ../src/pyproperty.cpp
  ../src/pysignal.cpp
  ../src/pyasync.cpp
  ../src/pyobjectfactory.cpp
  ../src/pylog.cpp
  ../src/pypath.cpp
  ../src/pytranslator.cpp
  ../src/error.cpp
  ../src/trace.cpp
)

qi_install_header(${_headers} SUBFOLDER qipython)

message(STATUS "Generating libs for python${pyver}")


qi_create_lib(qipython${pyver} SHARED
  ${_headers}
  ${_sources}
)

qi_use_lib(qipython${pyver} BOOST_PYTHON${pyver} PYTHON${pyver} QI QITYPE QIMESSAGING )

qi_stage_lib(qipython${pyver})


# do not use qi_create_lib: it does not all
# the python-specific stuff we need.
# FIXME: use qi_create_py_ext() from qibuild
# when it is implemented
add_library(_qi${pyver} SHARED ../src/boostpymain.cpp)

#you may ask why? ... because we want PYTHONPATH to work both for python2 and python3
#so if the two binary do not have the same name we can put then in the same folder.
if("${pyver}" STREQUAL "3")
  set_target_properties(_qi${pyver} PROPERTIES OUTPUT_NAME _qi.abi3)
endif()

# Fix the library name: no lib prefix, correct extension
set_target_properties(_qi${pyver} PROPERTIES PREFIX "")
if(UNIX)
  set_target_properties(_qi${pyver} PROPERTIES
    SUFFIX ".so"
    LIBRARY_OUTPUT_DIRECTORY ${QI_SDK_DIR}/${QI_SDK_LIB}
  )
endif()

#set rpath so that _qi can find its dependencies when loaded.
if(UNIX AND NOT APPLE)
  # Use a relative rpath at installation
  set_target_properties(_qi${pyver}
    PROPERTIES
      INSTALL_RPATH "\$ORIGIN/../lib"
  )
endif()

if (WIN32)
  set_target_properties(_qi${pyver} PROPERTIES
    SUFFIX ".pyd"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${QI_SDK_DIR}/${QI_SDK_LIB}/
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${QI_SDK_DIR}/${QI_SDK_LIB}/
  )
endif()

qi_use_lib(_qi${pyver} QIPYTHON${pyver})

# Install the library next to the .py files
qi_install_python(TARGETS _qi${pyver} VERSION ${pyver})
qi_install_python(${CMAKE_CURRENT_SOURCE_DIR}/../qi VERSION ${pyver} KEEP_RELATIVE_PATHS)

# Add automatic testing for Python bindings (for each versions)
add_subdirectory(../test dstonpython${pyver})
