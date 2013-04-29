
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../)

qi_install_python(${CMAKE_CURRENT_SOURCE_DIR}/qi
  KEEP_RELATIVE_PATHS
)

qi_create_lib(_qi src/boostpymain.cpp
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
                   src/pysignal.cpp
                   src/pysignal.hpp
                   src/gil.hpp)

# Fix the library name: no lib prefix, correct extension
set_target_properties(_qi PROPERTIES PREFIX "")
if(APPLE)
  set_target_properties(_qi PROPERTIES SUFFIX ".so")
endif()

if (WIN32)
  set_target_properties(_qi PROPERTIES SUFFIX ".pyd")
endif()

qi_use_lib(_qi BOOST_PYTHON QI QITYPE QIMESSAGING PYTHON)


# Add automatic testing for Python bindings
add_subdirectory(test)
