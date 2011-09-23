#!/bin/sh
##
## Copyright (C) 2011 Aldebaran Robotics

# Replace AL::ALPtr* by their boost equivalent

replacer $@ "(\s*#\s*include\s+)[\"<]\s*alcore/alptr.h[\">]" "\1<boost/shared_ptr.hpp>"
replacer $@ "AL::ALPtr" "boost::shared_ptr"
replacer $@ "ALPtr"     "boost::shared_ptr"
replacer $@ "AL::ALWeakPtr" "boost::weak_ptr"
replacer $@ "ALWeakPtr"     "boost::weak_ptr"
