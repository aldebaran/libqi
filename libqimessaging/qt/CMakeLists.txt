cmake_minimum_required(VERSION 2.6.4 FATAL_ERROR)

project(QtQiMessaging)

include(qibuild/modules/qt-tools)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

set(H_TO_MOC
        qimessaging/qt/qitransportsocket.h
        src/qitransportsocket_p.h
        qimessaging/qt/qitransportserver.h
        src/qitransportserver_p.h
        qimessaging/qt/qigateway.h
        src/qigateway_p.h
        qimessaging/qt/qisession.h)

set(H   qimessaging/qt/api.h
        qimessaging/qt/qidatastream.h
        qimessaging/qt/qiserver.h
        qimessaging/qt/QiTransportSocket
        qimessaging/qt/qitransportsocket.h
        qimessaging/qt/QiTransportServer
        qimessaging/qt/qitransportserver.h
        qimessaging/qt/QiGateway
        qimessaging/qt/qigateway.h
        qimessaging/qt/qisession.h)

set(H_P src/qiremoteobject_p.h
        src/qisession_p.h
        src/qmetaobjectbuilder_p.h
        src/qimetatype_p.h
        src/qitransportsocket_p.h
        src/qitransportserver_p.h
        src/qigateway_p.h
        src/qiqobject.h
        )

set(SRC src/qisession.cpp
        src/qiserver.cpp
        src/qiremoteobject.cpp
        src/qmetaobjectbuilder.cpp
        src/qimetatype.cpp
        src/qitransportsocket.cpp
        src/qitransportserver.cpp
        src/qigateway.cpp
        src/qiqobject.cpp)


qt4_wrap_cpp(H_MOC ${H_TO_MOC})

qi_create_lib(qimessaging-qt ${H_P} ${H} ${SRC} ${H_MOC})

qi_use_lib(qimessaging-qt qimessaging QT_QTCORE QT_QTNETWORK)

qi_install_header(${H} KEEP_RELATIVE_PATHS)
qi_stage_lib(qimessaging-qt)

add_subdirectory("bin")
add_subdirectory("test")
