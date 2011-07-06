/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2011 Aldebaran Robotics
*/

#include <iostream>
#include <qi/time.hpp>
#include <qi/rt.hpp>
#include <pthread.h>

void mythread(void *data)
{
  qi::time::period  period;
  qi::time::counter counter;

  period.init();
  while (true) {
    counter.start();
    std::cout << "c:" << counter << std::endl;
    counter.stop();
    //do the fucking work
    period.wait(true);
    std::cout << "p:" << period << std::endl;
  }
  return 32;
}


int main() {
  pthread_t *thd;

  int ret = qi::rt::realtime_thread_create(thd, qi::rt::SCHED_FIFO, 45, &mythread, 0);
  if (!ret) {
    ret = pthread_create(thd, 0, 0, mythread, 0);
    if (!ret)
      return 2;
  }
  thd.join();
}
