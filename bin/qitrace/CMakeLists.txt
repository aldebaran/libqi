##
## Copyright (C) 2013 Aldebaran Robotics
##

project (QiTrace)

qi_create_bin(qitrace
  qitrace.cpp
  DEPENDS QI QIMESSAGING BOOST_PROGRAM_OPTIONS)

