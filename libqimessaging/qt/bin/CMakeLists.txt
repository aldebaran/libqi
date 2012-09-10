##
## Author(s):
##  - Laurent LEC        <llec@aldebaran-robotics.com>
##
## Copyright (C) 2012 Aldebaran Robotics
##

project (QiQt)

qi_create_bin(qi-gateway-qt qt-gateway.cpp DEPENDS QI QIMESSAGING)
qi_use_lib(qi-gateway-qt qimessaging-qt QT_QTCORE)

add_subdirectory("qi-qt")
add_subdirectory("qiservice-qt")
