/*
**
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2011 Aldebaran Robotics
*/

#include <qi/os.hpp>
#include <sys/timeb.h>
#include <winsock2.h>

namespace qi {
  namespace os {

// A global time provider is needed, as the performance counter
// is relative to system start time, which we get only once
struct TimeStorage {
  bool                   usePerformanceCounter;
  qi::os::timeval        systemStartTime;
  LARGE_INTEGER          systemStartTicks;
  LARGE_INTEGER          systemTicksPerSecond;
};

static TimeStorage *gTimeStorage;

static void init_timer()
{
  if (gTimeStorage)
    return;
  gTimeStorage = new TimeStorage;

  // get the system clock frequency
  // in theory this never changes, but advanced
  // power saving modes on laptops may change this
  // TODO investigate if this is a problem for us
  int success = QueryPerformanceFrequency( &gTimeStorage->systemTicksPerSecond );
  gTimeStorage->usePerformanceCounter = false;
  // See http://www.nynaeve.net/?p=108 : colorburst crystal: good. Everything else: bad.
  if (success/* && (systemTicksPerSecond.QuadPart == 1193182 || systemTicksPerSecond.QuadPart == 3579545)*/)
  {
    gTimeStorage->usePerformanceCounter = true;

    // Get the system ticks at startup (test using CPU affinity)
    // Thread affinity coped from an MS example
    DWORD_PTR oldmask = SetThreadAffinityMask(GetCurrentThread(), 0);
    QueryPerformanceCounter(&gTimeStorage->systemStartTicks);
    SetThreadAffinityMask(GetCurrentThread(), oldmask);
  }

  // get the start time
  struct _timeb timebuffer;
  _ftime64_s( &timebuffer );

  // store this in a timeval struct
  gTimeStorage->systemStartTime.tv_sec  = (long)timebuffer.time;
  gTimeStorage->systemStartTime.tv_usec = 1000*timebuffer.millitm;
}

/**
 * Special Hack for windows using performance counter
 * to give accurate timing, otherwise the accuracy is only +/- 16ms
 * @return Always zero
 */
int gettimeofday(qi::os::timeval *t, void *)
{
  if (!gTimeStorage)
    init_timer();
  if (gTimeStorage->usePerformanceCounter) {
    LARGE_INTEGER lCurrentSystemTicks;

    // gets the elapsed ticks since system startup (test using CPU affinity)
    DWORD_PTR oldmask = SetThreadAffinityMask(GetCurrentThread(), 0);
    QueryPerformanceCounter(&lCurrentSystemTicks);
    SetThreadAffinityMask(GetCurrentThread(), oldmask);

    // remove the initial offset
    lCurrentSystemTicks.QuadPart -= gTimeStorage->systemStartTicks.QuadPart;

    // convert to a double number of seconds, using the ticksPerSecond
    double secondsElapsedDouble = ((double)lCurrentSystemTicks.QuadPart) /
      ((double)gTimeStorage->systemTicksPerSecond.QuadPart);

    // convert to the parts needed for the timeval
    long seconds = long(secondsElapsedDouble);
    long microseconds = long((secondsElapsedDouble - seconds) * 1000000);

    // add this offset to system startup time
    t->tv_sec  = gTimeStorage->systemStartTime.tv_sec  + seconds;
    t->tv_usec = gTimeStorage->systemStartTime.tv_usec + microseconds;
  } else {
    // no good performance counter, so revert to old behaviour
    struct _timeb timebuffer;
    _ftime64_s( &timebuffer );

    // store this in a timeval struct
    t->tv_sec=(long)timebuffer.time;
    t->tv_usec=1000*timebuffer.millitm;
  }
  return 0;
}

}
}
