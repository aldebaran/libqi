cmake_minimum_required(VERSION 2.6.4 FATAL_ERROR)

project(QtQiMessaging)

include(qibuild/modules/qt-tools)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

set(H_TO_MOC
        qimessaging/qt/qisession.h)

set(H   qimessaging/qt/qiserver.h
        qimessaging/qt/qitransportserver.h
        qimessaging/qt/qitransportsocket.h
        qimessaging/qt/qisession.h)

set(H_P src/qiremoteobject_p.h
        src/qisession_p.h
        src/qmetaobjectbuilder_p.h
        src/qimetatype_p.h)

set(SRC src/qisession.cpp
        src/qiserver.cpp
        src/qitransportserver.cpp
        src/qitransportsocket.cpp
        src/qiremoteobject.cpp
        src/qmetaobjectbuilder.cpp
        src/qimetatype.cpp)


qt4_wrap_cpp(H_MOC ${H_TO_MOC})

qi_create_lib(qimessaging-qt ${H_P} ${H} ${SRC} ${H_MOC})

qi_use_lib(qimessaging-qt qimessaging QT_QTCORE QT_QTNETWORK)

qi_stage_lib(qimessaging-qt)
