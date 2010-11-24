##
## Author(s):
##  - Cedric GESTES      <gestes@aldebaran-robotics.com>
##  - Chris Kilner       <ckilner@aldebaran-robotics.com>
##
## Copyright (C) 2009, 2010 Aldebaran Robotics
##

project (QiTools)

create_bin(qitools qimain.cpp)
set_target_properties(qitools PROPERTIES OUTPUT_NAME qi)
use_lib(qitools QI BOOST_PROGRAM_OPTIONS)
