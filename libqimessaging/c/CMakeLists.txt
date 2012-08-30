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
                                      qimessaging/c/session_c.h
                                      qimessaging/c/future_c.h
                                      qimessaging/c/signature_c.h
                                      qimessaging/c/application_c.h

                                      src/message_c_p.h
                                      src/object_c_p.h
                                      src/future_c_p.h
                                      src/message_c.cpp
                                      src/session_c.cpp
                                      src/object_c.cpp
                                      src/future_c.cpp
                                      src/application_c.cpp
                                      src/signature_c.cpp)
