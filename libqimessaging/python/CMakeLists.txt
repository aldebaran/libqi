


cmake_minimum_required(VERSION 2.6)

project(PyQiMessaging)

#use(PYTHON-TOOLS)
include("qibuild/swig/python")
qi_swig_wrap_python(qi src/qi.i SRC src/qipython.cpp src/qipython.hpp DEPENDS QI QIMESSAGING)
