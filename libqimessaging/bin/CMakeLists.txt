##
## Author(s):
##  - Cedric GESTES      <gestes@aldebaran-robotics.com>
##  - Chris Kilner       <ckilner@aldebaran-robotics.com>
##  - Laurent LEC        <llec@aldebaran-robotics.com>
##
## Copyright (C) 2009, 2010, 2012 Aldebaran Robotics
##

add_subdirectory("qicli")
add_subdirectory("qiclient")
add_subdirectory("qiservice")
add_subdirectory("qimaster")
add_subdirectory("qigateway")
if(WITH_QT_QTCORE)
  add_subdirectory("qi-qt")
  add_subdirectory("qiservice-qt")
endif()
add_subdirectory("qilb")
add_subdirectory("qiclient-c")
add_subdirectory("qiservice-c")
