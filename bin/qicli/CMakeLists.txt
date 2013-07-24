##
## Author(s):
##  - Laurent LEC        <llec@aldebaran-robotics.com>
##
## Copyright (C) 2012 Aldebaran Robotics
##

project (QiCLI)

set(SRCS
  qicli.hpp
  sessionhelper.hpp
  servicehelper.hpp
  sessionhelper.cpp
  servicehelper.cpp
  main.cpp
  subcmds.cpp
  utils.cpp)
  qitrace.cpp

qi_create_bin(qicli
  ${SRCS}
  DEPENDS QI QIMESSAGING BOOST_PROGRAM_OPTIONS)

include_directories(./)
