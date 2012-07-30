##
## Author(s):
##  - Laurent LEC        <llec@aldebaran-robotics.com>
##
## Copyright (C) 2012 Aldebaran Robotics
##

project (QiLB)

qi_add_optional_package(SQLITE3)
if(NOT WITH_SQLITE3)
  return()
endif()

qi_create_bin(qi-lb
  main.cpp
  DEPENDS QI QIMESSAGING SQLITE3)

