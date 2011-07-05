#ifndef RT_HPP
#define RT_HPP

#include <ctime>

namespace qi {

namespace time {

unsigned long long second(     const struct timespec &ts);
unsigned long long millisecond(const struct timespec &ts);
unsigned long long nanosecond( const struct timespec &ts);
unsigned long long microsecond(const struct timespec &ts);

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

class period : public timer {
public:
  period();
  period(const clock &default_clock);
  set_period(struct timespec *ts);
  init();
  wait(bool allow_drifting = true);
};

}
}

#endif // RT_HPP
