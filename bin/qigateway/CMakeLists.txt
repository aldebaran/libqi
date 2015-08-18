##
## Author(s):
##  - Cedric GESTES      <gestes@aldebaran-robotics.com>
##  - Herve CUCHE        <hcuche@aldebaran-robotics.com>
##
## Copyright (C) 2009, 2010, 2012 Aldebaran Robotics
##

project (QiGateway)

if(DEFINED ENV{BUILD_TAG})
  set(_build_tag $ENV{BUILD_TAG})
  add_definitions(" -DGATEWAY_BUILD_TAG=\"\\\"gateway-${_build_tag}\\\"\" ")
endif()

qi_add_optional_package(BREAKPAD)

qi_create_bin(qi-gateway
  qigatewaymain.cpp
  DEPENDS
  QI
  BOOST_PROGRAM_OPTIONS
  )

if(WITH_BREAKPAD)
  add_definitions(" -DWITH_BREAKPAD ")
  add_definitions(" -DBREAKPAD_DUMP_DIR=\"\\\"/var/lib/minidump\\\"\" ")
  qi_info("Breakpad utilities found. Enabling minidump generation.")
  qi_use_lib(qi-gateway BREAKPAD)
else()
  qi_info("Breakpad utilities not found. Disabling minidump generation.")
endif()
