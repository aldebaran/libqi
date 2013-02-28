##
## Author(s):
##  - Cedric GESTES <gestes@aldebaran-robotics.com>
##  - Nicolas Cornu <ncornu@aldebaran-robotics.com>
##
## Copyright (C) 2010, 2011, 2012 Aldebaran Robotics
##

qi_create_perf_test(perf_transport perf_transport.cpp
  DEPENDS
    QI QIMESSAGING QIPERF BOOST_PROGRAM_OPTIONS)

  #qi_create_perf_test(perf_transport_thd perf_transport_thd.cpp
  #  DEPENDS
  #    QI QIMESSAGING QIPERF BOOST_PROGRAM_OPTIONS)

#qi_create_gtest(perf_maps    SRC perf_maps.cpp    DEPENDS QI GTEST TIMEOUT 120)
#add_subdirectory(messaging)
#qi_create_gtest(perf_functor SRC perf_functor.cpp DEPENDS QI GTEST TIMEOUT 120)
#qi_create_bin(perf_type            SRC dataperftimer.* perf_type.cpp            DEPENDS QI QIMESSAGING BOOST_SIGNALS NO_INSTALL )
#qi_create_bin(perf_pool            SRC dataperftimer.* perf_pool.cpp            DEPENDS QI QIMESSAGING NO_INSTALL)
#qi_create_bin(perf_transport_event SRC dataperftimer.* perf_transport_event.cpp DEPENDS QI QIMESSAGING NO_INSTALL)

qi_create_perf_test(perf_qi_signal_direct perf_qi_signal_direct.cpp
  DEPENDS
    QI QIPERF QITYPE BOOST_PROGRAM_OPTIONS)

# qi_create_perf_test(perf_qi_signal_queued perf_qi_signal_queued.cpp
#   DEPENDS
#     QI QIPERF BOOST_PROGRAM_OPTIONS
#     ARGUMENTS "--backend=codespeed")

qi_create_perf_test(perf_boost_signal perf_boost_signal.cpp
  DEPENDS
    QI QIPERF QITYPE BOOST_PROGRAM_OPTIONS BOOST_SIGNALS)

qi_create_perf_test(perf_boost_function perf_boost_function.cpp
  DEPENDS
    QI QIPERF QITYPE BOOST_PROGRAM_OPTIONS)

qi_create_perf_test(perf_function perf_function.cpp
  DEPENDS
    QI QIPERF QITYPE BOOST_PROGRAM_OPTIONS)

qi_create_perf_test(perf_create_service perf_create_service.cpp
  DEPENDS
    QI QIPERF QITYPE QIMESSAGING BOOST_PROGRAM_OPTIONS)
