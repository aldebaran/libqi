##
## Copyright (C) 2008 Aldebaran Robotics
##

project(tinyxml)

set(TINYXML_H
  tinyxml/tinyxml.h
  tinyxml/tinystr.h
)

set( TINYXML_SRCS
  tinyxml/tinystr.cpp
  tinyxml/tinyxml.cpp
  tinyxml/tinyxmlerror.cpp
  tinyxml/tinyxmlparser.cpp
 )

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
create_lib(tinyxml ${TINYXML_SRCS} ${TINYXML_H})
install_header(TINYXML SUBFOLDER tinyxml ${TINYXML_H})
stage_lib(tinyxml TINYXML)

