##
## Copyright (C) 2008, 2011 Aldebaran Robotics
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
qi_create_lib(tinyxml ${TINYXML_SRCS} ${TINYXML_H})
qi_install_header(SUBFOLDER tinyxml ${TINYXML_H})
qi_stage_lib(tinyxml)

