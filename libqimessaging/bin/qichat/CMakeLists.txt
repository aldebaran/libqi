##
## Author(s):
##  - Cedric GESTES      <gestes@aldebaran-robotics.com>
##  - Chris Kilner       <ckilner@aldebaran-robotics.com>
##
## Copyright (C) 2009, 2010 Aldebaran Robotics
##

qi_create_bin(qichat-display qichat-display.cpp
              DEPENDS QI QIMESSAGING BOOST_PROGRAM_OPTIONS)

qi_create_bin(qichat-prompt qichat-prompt.cpp
              DEPENDS QI QIMESSAGING BOOST_PROGRAM_OPTIONS)
