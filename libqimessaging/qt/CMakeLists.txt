cmake_minimum_required(VERSION 2.6.4 FATAL_ERROR)

project(QtQiMessaging)

include(qibuild/modules/qt-tools)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

set(H_TO_MOC
        qimessaging/qisession.h)

set(H   qimessaging/qiserver.h
        qimessaging/qisession.h)

set(H_P src/qiremoteobject_p.h
        src/qmetaobjectbuilder_p.h)

set(SRC src/qisession.cpp
        src/qiserver.cpp
        src/qiremoteobject.cpp
        src/qmetaobjectbuilder.cpp)


qt4_wrap_cpp(H_MOC ${H_TO_MOC})

qi_create_lib(qimessaging-qt ${H_P} ${H} ${SRC} ${H_MOC})

qi_use_lib(qimessaging-qt qimessaging QT_QTCORE)

qi_stage_lib(qimessaging-qt)
