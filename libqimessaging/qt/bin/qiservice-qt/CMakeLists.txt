##
## Author(s):
##  - Laurent LEC        <llec@aldebaran-robotics.com>
##
## Copyright (C) 2012 Aldebaran Robotics
##

project (QiQtService)

include(qibuild/modules/qt-tools)

set(H_TO_MOC helloservice.h)

qt4_wrap_cpp(H_MOC ${H_TO_MOC})

qi_create_bin(qi-service-qt
    qiqtservicetestmain.cpp
    helloservice.cpp ${H_MOC}
    DEPENDS QI QIMESSAGING BOOST_PROGRAM_OPTIONS)


qi_use_lib(qi-service-qt qimessaging-qt QT_QTCORE)
