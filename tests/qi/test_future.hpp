#ifndef QI_TEST_FUTURE_HPP
#define QI_TEST_FUTURE_HPP

#include <thread>
#include <qi/future.hpp>
#include <qi/log.hpp>
#include <qi/os.hpp>

class SetValue: private boost::noncopyable
{
public:
  SetValue(std::atomic<int>& tgt);
  int exchange(int v);
  int delayExchange(qi::MilliSeconds delay, int value);
  std::atomic<int>& target;
  std::atomic<int> state;
};

class SetValue2: public SetValue, public qi::Trackable<SetValue2>
{
public:
  SetValue2(std::atomic<int>& target);
  ~SetValue2();
  void delayExchangeP(qi::MilliSeconds delay, int value, qi::Promise<int> result);
};

int block(int i, qi::Future<void> f);
int get42();


#endif // QI_TEST_FUTURE_HPP
