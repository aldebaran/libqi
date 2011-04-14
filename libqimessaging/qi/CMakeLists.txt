##
## Author(s):
##  - Cedric GESTES      <gestes@aldebaran-robotics.com>
##  - Chris Kilner       <ckilner@aldebaran-robotics.com>
##
## Copyright (C) 2009, 2010, 2011 Aldebaran Robotics
##


qi_submodule_create("qi-core"            "core/*.h[px][px]")

qi_submodule_create("qi-exceptions"      "exceptions/exceptions.hpp")

qi_submodule_create("qi-messaging"       "messaging/*.h[px][px]" "messaging.hpp"
                                         "messaging/*.cpp" "messaging/src/*.[ch][px][px]")

qi_submodule_create("qi-perf"            "perf/*.h[px][px]"
                                         "perf/*.cpp")

qi_submodule_create("qi-transport"       "transport/*.[ch][px][px]")


qi_submodule_add(qi-messaging "messaging/src/network/*.[ch][px][px]")

qi_submodule_add(qi-messaging
                 transport/src/zmq/zmq_client_backend.cpp
                 transport/src/zmq/zmq_client_backend.hpp
                 transport/src/zmq/zmq_connection_handler.cpp
                 transport/src/zmq/zmq_connection_handler.hpp
                 transport/src/zmq/zmq_forwarder_backend.cpp
                 transport/src/zmq/zmq_forwarder_backend.hpp
                 transport/src/zmq/zmq_poll_client.cpp
                 transport/src/zmq/zmq_poll_client.hpp
                 transport/src/zmq/zmq_publisher.cpp
                 transport/src/zmq/zmq_publisher.hpp
                 transport/src/zmq/zmq_simple_server_backend.cpp
                 transport/src/zmq/zmq_simple_server_backend.hpp
                 transport/src/zmq/zmq_subscriber.cpp
                 transport/src/zmq/zmq_subscriber.hpp
                 IF WITH_ZEROMQ
                 DEPENDS ZEROMQ)

# qi_submodule_add(qi-serialization SRC qi/serialization/thrift
#                     IF WITH_THRIFT
#                     DEPEND thrift)

# qi_submodule_add(qi-serialization SRC qi/serialization/boost
#                     IF WITH_BOOST_SERIALIZATION
#                     DEPEND boost_serialization)



