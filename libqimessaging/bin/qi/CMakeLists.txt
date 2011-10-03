##
## Author(s):
##  - Cedric GESTES      <gestes@aldebaran-robotics.com>
##  - Chris Kilner       <ckilner@aldebaran-robotics.com>
##
## Copyright (C) 2009, 2010 Aldebaran Robotics
##

project (QiTools)

qi_create_bin(qitools qimain.cpp DEPENDS QI QIMESSAGING BOOST_PROGRAM_OPTIONS)
set_target_properties(qitools PROPERTIES OUTPUT_NAME qi)
