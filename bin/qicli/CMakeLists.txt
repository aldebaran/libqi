##
## Author(s):
##  - Laurent LEC        <llec@aldebaran-robotics.com>
##
## Copyright (C) 2012 Aldebaran Robotics
##

project (QiCLI)

qi_create_bin(qicli
  qicli.cpp
  DEPENDS QI QIMESSAGING BOOST_PROGRAM_OPTIONS)

