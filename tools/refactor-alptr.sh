#!/bin/sh
##
## Author(s):
##  - Cedric GESTES <gestes@aldebaran-robotics.com>
##
## Copyright (C) 2011 Aldebaran Robotics
##

replacer $@ "(\s*#\s*include\s+)[\"<]\s*alcore/alptr.h[\">]" "\1<boost/shared_ptr.hpp>"
replacer $@ "AL::ALPtr" "boost::shared_ptr"
replacer $@ "ALPtr"     "boost::shared_ptr"
replacer $@ "AL::ALWeakPtr" "boost::weak_ptr"
replacer $@ "ALWeakPtr"     "boost::weak_ptr"
