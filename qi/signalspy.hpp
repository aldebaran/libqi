#pragma once

#ifndef _QI_SIGNALHELPER_HPP_
#define _QI_SIGNALHELPER_HPP_

#include <qi/signal.hpp>
#include <qi/anyobject.hpp>
#include <qi/trackable.hpp>
#include <boost/lambda/lambda.hpp>

namespace qi
{

class SignalSpy: public qi::Trackable<SignalSpy>
{
public:
  template <typename T>
  SignalSpy(SignalF<T>& signal)
    : _counter(0)
  {
    signal.connect(&SignalSpy::counterCallback, this);
  }

  SignalSpy(qi::AnyObject& object, const std::string& signalName)
    : _counter(0)
  {
    object.connect(signalName,
                   qi::AnyFunction::fromDynamicFunction(qi::bind(&SignalSpy::counterCallback, this)));
  }

  ~SignalSpy()
  {
    destroy();
  }

  unsigned int getCounter() const
  {
    boost::mutex::scoped_lock lock(_mutex);
    return _counter;
  }

  bool waitUntil(unsigned int nbTriggers, qi::Duration timeout) const
  {
    boost::mutex::scoped_lock lock(_mutex);
    return _cond.wait_for(lock, timeout, boost::function<bool()>(boost::lambda::var(_counter) >= nbTriggers));
  }

private:
  mutable boost::mutex _mutex;
  mutable boost::condition_variable _cond;
  unsigned int _counter;

  qi::AnyReference counterCallback()
  {
    boost::mutex::scoped_lock lock(_mutex);
    ++_counter;
    _cond.notify_all();
    return qi::AnyReference(qi::typeOf<void>());
  }
};
}

#endif
