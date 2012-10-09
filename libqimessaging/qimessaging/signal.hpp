#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QIMESSAGING_SIGNAL_HPP_
#define _QIMESSAGING_SIGNAL_HPP_

#include <qimessaging/eventloop.hpp>
#include <qimessaging/signature.hpp>
#include <qimessaging/functiontype.hpp>

namespace qi {

  class ObjectInterface;
  class ManageablePrivate;
  struct SignalSubscriber;
 /** User classes can inherit from Manageable to benefit from additional features:
  * - Automatic signal disconnection when the object is deleted
  * - Event loop management
  */
 class QIMESSAGING_API Manageable
 {
 public:
   Manageable();
   ~Manageable();
   Manageable(const Manageable& b);
   void operator = (const Manageable& b);

   void addCallbacks(ObjectInterface *callbacks, void *data = 0);
   void removeCallbacks(ObjectInterface *callbacks);

   // Remember than this is the target of subscriber
   void addRegistration(const SignalSubscriber& subscriber);
   // Notify that a registered subscriber got disconnected
   void removeRegistration(unsigned int linkId);

   EventLoop* eventLoop() const;
   void moveToEventLoop(EventLoop* eventLoop);

   ManageablePrivate* _p;
 };

  class GenericObject;
  typedef boost::shared_ptr<GenericObject> ObjectPtr;
  class SignalBasePrivate;
  struct SignalSubscriber;

  class QIMESSAGING_API SignalBase
  {
  public:
    explicit SignalBase(const std::string& signature);
    SignalBase();
    ~SignalBase();
    SignalBase(const SignalBase& b);
    SignalBase& operator = (const SignalBase& b);
    virtual std::string signature() const;

    typedef unsigned int Link;

    template<typename FUNCTION_TYPE>
    Link connect(FUNCTION_TYPE f, EventLoop* ctx = getDefaultObjectEventLoop());

    Link connect(qi::ObjectPtr target, unsigned int slot);
    Link connect(GenericFunction callback, EventLoop* ctx = getDefaultObjectEventLoop());
    Link connect(const SignalSubscriber& s);

    /** Disconnect a SignalHandler. The associated callback will not be called
     * anymore as soon as this function returns, but might be called in an
     * other thread while this function runs.
     */
    bool disconnect(const Link& link);

    void trigger(const GenericFunctionParameters& params);
    void operator()(
      qi::AutoGenericValue p1 = qi::AutoGenericValue(),
      qi::AutoGenericValue p2 = qi::AutoGenericValue(),
      qi::AutoGenericValue p3 = qi::AutoGenericValue(),
      qi::AutoGenericValue p4 = qi::AutoGenericValue(),
      qi::AutoGenericValue p5 = qi::AutoGenericValue(),
      qi::AutoGenericValue p6 = qi::AutoGenericValue(),
      qi::AutoGenericValue p7 = qi::AutoGenericValue(),
      qi::AutoGenericValue p8 = qi::AutoGenericValue());

    std::vector<SignalSubscriber> subscribers();
  public:
    boost::shared_ptr<SignalBasePrivate> _p;
  };

  template<typename FUNCTION_TYPE>
  inline SignalBase::Link SignalBase::connect(FUNCTION_TYPE  callback, EventLoop* ctx)
  {
    return connect(makeGenericFunction(callback), ctx);
  }

  template<typename T>
  class Signal: public SignalBase, public boost::function<T>
  {
  public:
    Signal();
    Signal(const Signal<T>& b);
    virtual std::string signature() const;
    using boost::function<T>::operator();
    inline SignalBase::Link connect(boost::function<T> f, EventLoop* ctx=getDefaultObjectEventLoop())
    {
      return SignalBase::connect(f, ctx);
    }
    inline SignalBase::Link connect(GenericFunction f, EventLoop* ctx=getDefaultObjectEventLoop())
    {
      return SignalBase::connect(f, ctx);
    }
    inline SignalBase::Link connect(qi::ObjectPtr target, unsigned int slot)
    {
      return SignalBase::connect(target, slot);
    }
  };

}

#include <qimessaging/details/signal.hxx>

QI_NO_TYPE(qi::SignalBase)

#endif  // _QIMESSAGING_SIGNAL_HPP_
