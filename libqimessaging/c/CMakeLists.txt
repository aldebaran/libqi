##
## Author(s):
##  - Cedric GESTES      <gestes@aldebaran-robotics.com>
##
## Copyright (C) 2012 Aldebaran Robotics
##

qi_submodule_create("qi-messaging-c"  qimessaging/c/api_c.h
                                      qimessaging/c/message_c.h
                                      qimessaging/c/object_c.h
                                      qimessaging/c/qi_c.h
                                      qimessaging/c/server_c.h
                                      qimessaging/c/session_c.h
                                      qimessaging/c/signature_c.h

                                      src/message_c.cpp
                                      src/server_c.cpp
                                      src/session_c.cpp
                                      src/signature_c.cpp)
