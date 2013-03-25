#project(PyQiMessaging)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../)
include("qibuild/swig/python")


file (GLOB SWIG_MODULE_qipy_EXTRA_DEPS
  ../c/qic/*.h
  ./src/*.hpp
)

# Compile swig compatibility layer
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/src")
qi_swig_wrap_python(qipy src/qipy.i SRC
                    src/qipython.hpp
                    src/qipython.cpp
                    src/pyobjectconverter.hpp
                    src/pyobjectconverter.cpp
                    src/pythonscopedref.hpp
                    DEPENDS QI QIMESSAGING QIC
)

qi_install_python(${CMAKE_CURRENT_SOURCE_DIR}/qi
  KEEP_RELATIVE_PATHS
)

# Add automatic testing for Python bindings
add_subdirectory(test)
