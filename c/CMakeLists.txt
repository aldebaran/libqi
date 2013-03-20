##
## Author(s):
##  - Cedric GESTES      <gestes@aldebaran-robotics.com>
##
## Copyright (C) 2012 Aldebaran Robotics
##

qi_create_lib("qic" SHARED qic/api.h
                     qic/application.h
                     qic/session.h
                     qic/object.h
                     qic/future.h
                     qic/servicedirectory.h
                     qic/value.h

                     src/object_p.h
                     src/object.cpp
                     src/future_p.h
                     src/future.cpp
                     src/session.cpp
                     src/application.cpp
                     src/value_p.h
                     src/value.cpp
                     src/servicedirectory.cpp)

qi_use_lib(qic qimessaging qitype qi)
qi_stage_lib(qic)
