#!/bin/sh
##
## Copyright (C) 2011 Aldebaran Robotics
##

# Temporary fix for qi::path API changes
# findConfiguration -> findConf and so on

replacer $@ 'qi::os::tmp[ \t]*\('   'qi::os::tmpdir('
