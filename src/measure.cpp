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
#include <cstdio>
#endif

namespace qi
{
  namespace measure
  {
    int getNumFD()
    {
      int fd_count = -1;

#ifdef __linux__
      char buf[64];

      snprintf(buf, 64, "/proc/%i/fd/", getpid());

      DIR *dir = opendir(buf);
      if (dir == NULL)
        return -1;

      while (readdir(dir) != NULL)
        ++fd_count;
      closedir(dir);
#endif

      return fd_count;
    }
  }
}
