##
## Author(s):
##  - Laurent LEC        <llec@aldebaran-robotics.com>
##
## Copyright (C) 2012 Aldebaran Robotics
##

project (QiUniversal)

qi_create_bin(qi-universal qi-universal.cpp DEPENDS QI QIMESSAGING BOOST_PROGRAM_OPTIONS)
set_target_properties(qi-universal PROPERTIES OUTPUT_NAME qi-universal)
