
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../)

qi_install_python(${CMAKE_CURRENT_SOURCE_DIR}/qi
  KEEP_RELATIVE_PATHS
)

qi_create_lib(qipy src/boostpymain.cpp
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
                   src/pysignal.hpp)

qi_use_lib(qipy BOOST_PYTHON QI QITYPE QIMESSAGING PYTHON)

# Add automatic testing for Python bindings
add_subdirectory(test)
