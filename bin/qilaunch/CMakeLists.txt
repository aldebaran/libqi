## Copyright (C) 2012 Aldebaran Robotics

project(QiLaunch)

qi_create_bin(qi-launch
  qilaunch.cpp
  DEPENDS QI BOOST_PROGRAM_OPTIONS
  )
