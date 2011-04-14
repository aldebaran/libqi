##
## Author(s):
##  - Cedric GESTES      <gestes@aldebaran-robotics.com>
##
## Copyright (C) 2010, 2011 Aldebaran Robotics
##

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

qi_submodule_create(qi-cbinding
                    qimessaging/signature.h
                    qimessaging/message.h
                    qimessaging/client.h
                    qimessaging/server.h
                    qimessaging/context.h
                    qimessaging/log.h
                    src/qi.cpp
                    src/signature.cpp
                    src/message.cpp
                    src/client.cpp
                    src/server.cpp
                    src/context.cpp
                    src/log.cpp)
