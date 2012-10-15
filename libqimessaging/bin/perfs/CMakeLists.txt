##
## Author(s):
##  - Cedric GESTES <gestes@aldebaran-robotics.com>
##  - Nicolas Cornu <ncornu@aldebaran-robotics.com>
##
## Copyright (C) 2010, 2011, 2012 Aldebaran Robotics
##

# Temporary function
function(_qimessaging_create_perf_bin name)
  qi_create_bin(${name}
    ${name}.cpp
    NO_INSTALL
    DEPENDS
    QI
    QIMESSAGING
    QIPERF
    BOOST_PROGRAM_OPTIONS
  )
endfunction()

_qimessaging_create_perf_bin(perf_transport)

_qimessaging_create_perf_bin(perf_qi_signal_queued)

_qimessaging_create_perf_bin(perf_boost_signal)
qi_use_lib(perf_boost_signal BOOST_SIGNALS)

_qimessaging_create_perf_bin(perf_boost_function)

_qimessaging_create_perf_bin(perf_function)

_qimessaging_create_perf_bin(perf_transport_socket)

_qimessaging_create_perf_bin(perf_transport_thd)

include(qibuild/modules/qt-tools)
qt4_wrap_cpp(_moc perf_qt_signal.hpp)
qi_create_bin(perf_qt_signal_direct perf_qt_signal_direct.cpp ${_moc} DEPENDS QI QIMESSAGING QIPERF BOOST_PROGRAM_OPTIONS qt_qtcore)
qi_use_lib(perf_qt_signal_direct QT_QTCORE)

#qi_create_gtest(perf_maps    SRC perf_maps.cpp    DEPENDS QI GTEST TIMEOUT 120)
#add_subdirectory(messaging)
#qi_create_gtest(perf_functor SRC perf_functor.cpp DEPENDS QI GTEST TIMEOUT 120)
#qi_create_bin(perf_type            SRC dataperftimer.* perf_type.cpp            DEPENDS QI QIMESSAGING BOOST_SIGNALS NO_INSTALL )
#qi_create_bin(perf_pool            SRC dataperftimer.* perf_pool.cpp            DEPENDS QI QIMESSAGING NO_INSTALL)
#qi_create_bin(perf_transport_event SRC dataperftimer.* perf_transport_event.cpp DEPENDS QI QIMESSAGING NO_INSTALL)


# --output is added by qi_create_perf_test().
# question: what to do with backends ?
qi_create_perf_test(perf_qi_signal_direct perf_qi_signal_direct.cpp
  DEPENDS
    QI QIMESSAGING QIPERF BOOST_PROGRAM_OPTIONS
    ARGUMENTS "--backend=codespeed")
