##
## Author(s):
##  - Cedric GESTES <gestes@aldebaran-robotics.com>
##
## Copyright (C) 2010, 2011, 2012 Aldebaran Robotics
##

#qi_create_gtest(perf_maps    SRC perf_maps.cpp    DEPENDS QI GTEST TIMEOUT 120)
#add_subdirectory(functors)
#add_subdirectory(maps)
#add_subdirectory(messaging)
#qi_create_gtest(perf_functor SRC perf_functor.cpp DEPENDS QI GTEST TIMEOUT 120)
qi_create_bin(perf_pool            SRC dataperftimer.* perf_pool.cpp            DEPENDS QI QIMESSAGING NO_INSTALL)
qi_create_bin(perf_transport       SRC dataperftimer.* perf_transport.cpp       DEPENDS QI QIMESSAGING NO_INSTALL)
qi_create_bin(perf_transport_socket SRC dataperftimer.* perf_transport_socket.cpp DEPENDS QI QIMESSAGING NO_INSTALL)
qi_create_bin(perf_transport_thd   SRC dataperftimer.* perf_transport_thd.cpp   DEPENDS QI QIMESSAGING NO_INSTALL)
qi_create_bin(perf_transport_event SRC dataperftimer.* perf_transport_event.cpp DEPENDS QI QIMESSAGING NO_INSTALL)
