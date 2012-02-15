##
## Author(s):
##  - Laurent LEC        <llec@aldebaran-robotics.com>
##
## Copyright (C) 2012 Aldebaran Robotics
##

project (QiQtService)

qi_create_bin(qi-service-qt
    qiqtservicetestmain.cpp
    DEPENDS QI QIMESSAGING BOOST_PROGRAM_OPTIONS)

qi_use_lib(qi-service-qt qimessaging-qt QT_QTCORE)
