## Copyright (C) 2011 Aldebaran Robotics

###############################################
# Auto-generated file.                        #
# Do not edit                                 #
# This file is part of the qibuild project    #
###############################################

set(QIBUILD_BOOTSTRAP_VERSION 9)

# Someone used qibuild and generated a dependencies.cmake
# file (for the dependencies and where to find qibuild/cmake file),
# so just use it.
if(EXISTS ${CMAKE_BINARY_DIR}/dependencies.cmake)
  include(${CMAKE_BINARY_DIR}/dependencies.cmake)
endif()

# Someone called cmake with a toolchain file that is
# able to find qibuild/cmake code, so just include it.
# Else, fail loudly.
include(qibuild/general)
