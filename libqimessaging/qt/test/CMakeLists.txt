include(qibuild/modules/qt-tools)

qi_create_gtest(test_Qt_QiTransportServer   SRC test_QiTransportServer.cpp    DEPENDS QI QIMESSAGING-QT QT_QTCORE GTEST    TIMEOUT 10)

qt4_wrap_cpp(_moc test_QiTransportSocket.hpp)
qi_create_gtest(test_Qt_QiTransportSocket   SRC test_QiTransportSocket.cpp ${_moc} DEPENDS QI QIMESSAGING-QT QT_QTCORE GTEST    TIMEOUT 10)
