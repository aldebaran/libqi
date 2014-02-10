/*
** Author(s):
** - Nicolas Cornu <ncornu@aldebaran-robotics.com>
**
** Copyright (C) 2013 Aldebaran Robotics
*/

#include <qiperf/measure.hpp>

#ifdef __linux__
#include <dirent.h>
#include <unistd.h>
#endif

namespace qi
{
  namespace measure
  {
    int getNumFD()
    {
      int fdCount = -1;

#ifdef __linux__
      DIR *dir = opendir("/proc/self/fd/");
      if (dir == NULL)
        return -1;

      while (readdir(dir) != NULL)
        ++fdCount;
      closedir(dir);
#endif

      return fdCount;
    }
  }
}
