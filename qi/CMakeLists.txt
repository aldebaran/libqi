##
## Author(s):
##  - Cedric GESTES      <gestes@aldebaran-robotics.com>
##  - Chris Kilner       <ckilner@aldebaran-robotics.com>
##
## Copyright (C) 2009, 2010 Aldebaran Robotics
##


# helper function to manage submodule and qi sources folders
function(submodule _submodule _name)
  #TODO: convert _name and _submodule => s!/!\\! for VSGROUP
  qi_submodule_create("${_submodule}" VSGROUP "${_submodule}\\${_name}"
                                      PUBLIC_HEADER "${_name}/*.hxx" "${_name}/*.hpp" "${_name}.hpp"
                                      SRC "${_name}/*.cpp" "${_name}/detail/*.cpp" "${_name}/detail/*.hxx" "${_name}/detail/*.hpp")
endfunction()

qi_submodule_create("qi-core"           PUBLIC_HEADER "core/*.h[px][px]")

qi_submodule_create("qi-exceptions"     PUBLIC_HEADER "exceptions/exceptions.hpp")

qi_submodule_create("qi-messaging"      PUBLIC_HEADER "messaging/*.h[px][px]" "messaging.hpp"
                                        SRC           "messaging/*.cpp" "messaging/src/*.[ch][px][px]")

qi_submodule_create("qi-serialization"  PUBLIC_HEADER "serialization/*.h[px][px]" "serialization.hpp"
                                        SRC           "serialization/*.cpp")

qi_submodule_create("qi-perf"           PUBLIC_HEADER "perf/*.h[px][px]"
                                        SRC           "perf/*.cpp")

qi_submodule_create("qi-log"            PUBLIC_HEADER "log/*.h[px][px]" "log.hpp"
                                        SRC           "log/*.cpp")

qi_submodule_create("qi-transport"      PUBLIC_HEADER "transport/*.[ch][px][px]")

qi_submodule_create("qi-functors"       PUBLIC_HEADER "functors/*.[ch][px][px]"
                                        SRC           "functors/detail/*.[hc][px][px]")

qi_submodule_create("qi-signature"      PUBLIC_HEADER "signature/*.[ch][px][px]" "signature.hpp"
                                        SRC           "signature/*.cpp" "signature/detail/*.[ch][px][px]")

qi_submodule_add(qi-transport VSGROUP "messaging\\src\\network" SRC "messaging/src/network/*.[ch][px][px]")

qi_submodule_add(qi-transport VSGROUP transport\\zmq
                 SRC
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

# qi_submodule_add(qi SRC qi/serialization/thrift
#                     VSGROUP serialization\\thrift
#                     IF WITH_THRIFT
#                     DEPEND thrift)

# qi_submodule_add(qi SRC qi/serialization/boost
#                     VSGROUP serialization\\boost
#                     IF WITH_BOOST_SERIALIZATION
#                     DEPEND boost_serialization)



