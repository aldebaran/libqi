cmake_minimum_required(VERSION 2.6.4 FATAL_ERROR)

project(QtQiMessaging)

include(qibuild/modules/qt-tools)

qt4_wrap_cpp(MOC_G qimessaging/qisession.h)

qi_create_lib(qimessaging-qt src/qisession.cpp qimessaging/*.h ${MOC_G})

qi_use_lib(qimessaging-qt qimessaging QT_QTCORE)

qi_stage_lib(qimessaging-qt)
