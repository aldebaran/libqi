##
## Author(s):
##  - Cedric GESTES      <gestes@aldebaran-robotics.com>
##  - Chris Kilner       <ckilner@aldebaran-robotics.com>
##
## Copyright (C) 2009, 2010 Aldebaran Robotics
##

project (qichat-display)
create_bin(qichat-display qichat-display.cpp)
use_lib(qichat-display QI BOOST_PROGRAM_OPTIONS)


project (qichat-prompt)
create_bin(qichat-prompt qichat-prompt.cpp)
use_lib(qichat-prompt QI BOOST_PROGRAM_OPTIONS)