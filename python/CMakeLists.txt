


cmake_minimum_required(VERSION 2.6)
project(swig-overlaod)

use(PYTHON-TOOLS)
wrap_python(qi qi.i DEPENDENCIES QI)
