#ifndef RT_HPP
#define RT_HPP

#include <ctime>
#include <iosfwd>

int mythread(int toto) {
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

namespace qi {

namespace time {

qi::os::timeval toTimeval(struct timespec &ts);

unsigned long long second(     const qi::os::timeval &ts);
unsigned long long millisecond(const qi::os::timeval &ts);
unsigned long long nanosecond( const qi::os::timeval &ts);
unsigned long long microsecond(const qi::os::timeval &ts);

qi::os::timeval second(     unsigned long long sec);
qi::os::timeval millisecond(unsigned long long ms);
qi::os::timeval microsecond(unsigned long long us);
qi::os::timeval nanosecond( unsigned long long ns);

class clock {
public:
  enum clock_type {
    realtime,
    monotonic,
    program_cpu_time,
    thread_cpu_time
  };

  /** \brief construct a clock, the type of the clock could be specified
    * throw qi::os_error is the clock is not available ?
    * fallback on monotonic?
    */
  clock(enum clock_type clocktype = realtime);

  //throw qi::os_error
  bool set_clock(enum clock_type clocktype);
  bool check_clock(enum clock_type clocktype)const;
  struct timespec get_time();
};

struct statistic {
  long iteration;
  struct timespec min_wait_ns;
  struct timespec max_wait_ns;
  struct timespec avg_wait_ns;
};

class counter {
public:
  counter();
  counter(const cclock &default_clock);
  setclock(const clock &new_clock)
  start();
  stop();

  //stop and reset statistics
  reset();

  //return stats (average..)
  struct statistic stat() const;

  //return the length of the last run
  struct timespec diff() const;
  unsigned long long diffns() const;
  unsigned long long diffus() const;
  unsigned long long diffms() const;

  //return current time - start time
  //could be used to get intermediate value between start/stop
  struct timespec currentdiff() const;
  unsigned long long currentdiffns() const;
  unsigned long long currentdiffus() const;
  unsigned long long currentdiffms() const;
};

class period : public counter {
public:
  period();
  period(const clock &default_clock);
  set_period(qi::os::timeval *ts);
  init();
  wait(bool allow_drifting = true);
};

}
}

std::ostream &operator<<(std::ostream &os) {
  //os << "sec:" <<
}

#endif // RT_HPP
