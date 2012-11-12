#project(PyQiMessaging)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../)
include("qibuild/swig/python")

# Compile swig compatibility layer
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/src")
qi_swig_wrap_python(qimessagingswig src/qimessagingswig.i SRC
                    src/qipython.hpp
                    src/qipython.cpp
                    src/qipython_call.hpp
                    src/qipython_call.cpp
                    src/qipython_convert.hpp
                    src/qipython_convert.cpp
                    src/sd_testwrap.hpp
                    src/sd_testwrap.cpp
                    DEPENDS QI QIMESSAGING)

# Add automatic testing for Python bindings
add_subdirectory(test)
