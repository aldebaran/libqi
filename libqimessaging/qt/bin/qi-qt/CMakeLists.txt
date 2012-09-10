##
## Author(s):
##  - Laurent LEC        <llec@aldebaran-robotics.com>
##
## Copyright (C) 2012 Aldebaran Robotics
##

project (QiQt)

qi_create_bin(qi-qt
    qiqtmain.cpp
    DEPENDS QI QIMESSAGING BOOST_PROGRAM_OPTIONS)

qi_use_lib(qi-qt qimessaging-qt QT_QTCORE)
