##
## Author(s):
##  - Laurent LEC        <llec@aldebaran-robotics.com>
##
## Copyright (C) 2012 Aldebaran Robotics
##

project (QiQtService)

include(qibuild/modules/qt-tools)

qt4_wrap_cpp(_moc helloservice.h)

qi_create_bin(qi-service-qt
    qiqtservicetestmain.cpp
    helloservice.cpp ${_moc}
    DEPENDS QI QIMESSAGING BOOST_PROGRAM_OPTIONS)


qi_use_lib(qi-service-qt qimessaging-qt QT_QTCORE)
