##
## Author(s):
##  - Laurent LEC        <llec@aldebaran-robotics.com>
##
## Copyright (C) 2012 Aldebaran Robotics
##

project (QiLB)

qi_create_bin(qilb
  main.cpp
  DEPENDS QI QIMESSAGING SQLITE3)

