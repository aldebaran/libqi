#!/bin/sh
##
## Copyright (C) 2011 Aldebaran Robotics

# Replace headers of alvision and alcore.

replacer $@ "(\s*#\s*include\s+)[\"<]\s*alvisiondefinitions.h[\">]"   "\1<alvision/alvisiondefinitions.h>"
replacer $@ "(\s*#\s*include\s+)[\"<]\s*alvideo.h[\">]"		      "\1<alvision/alvideo.h>"
replacer $@ "(\s*#\s*include\s+)[\"<]\s*alimage.h[\">]"		      "\1<alvision/alimage.h>"
replacer $@ "(\s*#\s*include\s+)[\"<]\s*alcore/alerror.h[\">]"	      "\1<alerror/alerror.h>"
replacer $@ "(\s*#\s*include\s+)[\"<]\s*alcore/alnetworkerror.h[\">]" "\1<alerror/alnetworkerror.h>"
