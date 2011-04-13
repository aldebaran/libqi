##
## Author(s):
##  - Dimitri Merejkowsky <dmerejkowsky@aldebaran-robotics.com>
##  - Cedric GESTES <gestes@aldebaran-robotics.com>
##
## Copyright (C) 2010 Aldebaran Robotics
##

cmake_minimum_required(VERSION 2.6.4)
project(RubyQiMessaging)

include(RUBY-TOOLSConfig.cmake)
wrap_ruby(Ruqi ruqi.i DEPENDENCIES QI)
