#pragma once

#ifndef _QI_SIGNALHELPER_HPP_
#define _QI_SIGNALHELPER_HPP_

#include <qi/signal.hpp>
#include <qi/anyobject.hpp>
#include <qi/trackable.hpp>

namespace qi
{

class SignalSpy: public qi::Trackable<SignalSpy>
{
public:
  template <typename T>
  SignalSpy(SignalF<T>& signal)
    : qi::Trackable<SignalSpy>(this)
    , counter(0)
  {
    signal.connect(&SignalSpy::counterCallback, this);
  }

  SignalSpy(qi::AnyObject& object, const std::string& signalName)
    : qi::Trackable<SignalSpy>(this)
    , counter(0)
  {
    object.connect(signalName,
                   qi::AnyFunction::fromDynamicFunction(qi::bind<qi::AnyReference(const qi::AnyReferenceVector&)>(&SignalSpy::counterCallback, this)));
  }

  ~SignalSpy()
  {
    destroy();
  }

  int getCounter() const
  {
    return *counter;
  }

private:
  qi::Atomic<int> counter;

  qi::AnyReference counterCallback()
  {
    ++counter;
    return qi::AnyReference(qi::typeOf<void>());
  }
};
}

#endif
