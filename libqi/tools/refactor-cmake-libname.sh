#!/bin/sh
##
## Author(s):
##  - Cedric GESTES <gestes@aldebaran-robotics.com>
##
## Copyright (C) 2011 Aldebaran Robotics
##

# Replace Aldebaran Library name

replacer $@ ALMEMORYFASTACCESS ALMEMORY_FAST_ACCESS --file-filter=CMakeLists.txt --file-filter="*.cmake"
replacer $@ LIBSHM             ALSHM                --file-filter=CMakeLists.txt --file-filter="*.cmake"
replacer $@ LIBLAUNCHER	       ALLAUNCHER           --file-filter=CMakeLists.txt --file-filter="*.cmake"
replacer $@ LIBCORE            ALCORE               --file-filter=CMakeLists.txt --file-filter="*.cmake"
replacer $@ TOOLS              ALTOOLS              --file-filter=CMakeLists.txt --file-filter="*.cmake"
replacer $@ LIBVISION          ALVISION             --file-filter=CMakeLists.txt --file-filter="*.cmake"
replacer $@ SHMPOOL	       ALSHMPOOL            --file-filter=CMakeLists.txt --file-filter="*.cmake"
replacer $@ LIBFILE	       ALFILE               --file-filter=CMakeLists.txt --file-filter="*.cmake"
replacer $@ LIBAUDIO	       ALAUDIO              --file-filter=CMakeLists.txt --file-filter="*.cmake"
replacer $@ LIBBEHAVIORINFO    ALBEHAVIORINFO       --file-filter=CMakeLists.txt --file-filter="*.cmake"
replacer $@ LIBRESOURCE	       ALRESOURCE           --file-filter=CMakeLists.txt --file-filter="*.cmake"
