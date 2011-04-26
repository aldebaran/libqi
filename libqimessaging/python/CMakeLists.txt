


cmake_minimum_required(VERSION 2.6)

project(PyQiMessaging)

#use(PYTHON-TOOLS)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../)
include("qibuild/swig/python")

file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/src")
qi_swig_wrap_python(qi src/qi.i SRC src/qipython.cpp src/qipython.hpp DEPENDS QI QIMESSAGING)
