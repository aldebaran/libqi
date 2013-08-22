
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../)

qi_install_python(${CMAKE_CURRENT_SOURCE_DIR}/qi
  KEEP_RELATIVE_PATHS
)

# do not use qi_create_lib: it does not all
# the python-specific stuff we need.
# FIXME: use qi_create_py_ext() from qibuild
# when it is implemented
add_library(_qi SHARED src/boostpymain.cpp
                   src/pyapplication.cpp
                   src/pyapplication.hpp
                   src/pysession.cpp
                   src/pysession.hpp
                   src/pyfuture.cpp
                   src/pyfuture.hpp
                   src/pyobject.cpp
                   src/pyobject.hpp
                   src/pyobjectconverter.cpp
                   src/pyproperty.cpp
                   src/pyproperty.hpp
                   src/pythreadsafeobject.hpp
                   src/pysignal.cpp
                   src/pysignal.hpp
                   src/pyservicedirectory.hpp
                   src/pyservicedirectory.cpp
                   src/pyobjectfactory.hpp
                   src/pyobjectfactory.cpp
                   src/gil.hpp
                   src/gil.cpp
                   src/error.hpp
)

# Install the library next to the .py files
qi_install_python(TARGETS _qi)

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

qi_use_lib(_qi BOOST_PYTHON QI QITYPE QIMESSAGING PYTHON)


# Add automatic testing for Python bindings
add_subdirectory(test)
