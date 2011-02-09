


cmake_minimum_required(VERSION 2.6)

project(PyQiMessaging)

#use(PYTHON-TOOLS)
include("qibuild/swig/python")
qi_swig_wrap_python(qi qi.i DEPENDENCIES QI)
