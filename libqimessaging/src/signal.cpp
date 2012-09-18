/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <map>
#include <qi/atomic.hpp>

#include <boost/thread/recursive_mutex.hpp>

#include <qimessaging/signal.hpp>
#include <qimessaging/genericvalue.hpp>

#include "src/object_p.hpp"
#include "src/signal_p.hpp"

namespace qi {


  static qi::atomic<long> linkUid = 1;

  void SignalBase::operator()(
      qi::AutoGenericValue p1,
      qi::AutoGenericValue p2,
      qi::AutoGenericValue p3,
      qi::AutoGenericValue p4,
      qi::AutoGenericValue p5,
      qi::AutoGenericValue p6,
      qi::AutoGenericValue p7,
      qi::AutoGenericValue p8)
  {
    qi::AutoGenericValue* vals[8]= {&p1, &p2, &p3, &p4, &p5, &p6, &p7, &p8};
    std::vector<qi::GenericValue> params;
    for (unsigned i=0; i<8; ++i)
      if (vals[i]->value)
        params.push_back(*vals[i]);
    // Signature construction
    std::string signature = "(";
    for (unsigned i=0; i< params.size(); ++i)
      signature += params[i].signature();
    signature += ")";
    if (signature != _p->signature)
    {
      qiLogError("qi.signal") << "Dropping emit: signature mismatch: " << signature <<" " << _p->signature;
      return;
    }
    trigger(MetaFunctionParameters(params, true));
  }

  void SignalBase::trigger(const MetaFunctionParameters& params)
  {
    boost::recursive_mutex::scoped_lock sl(_p->mutex);
    SignalSubscriberMap::iterator i;
    for (i=_p->subscriberMap.begin(); i!= _p->subscriberMap.end(); ++i)
      i->second.call(params);
  }

  static void functor_call(MetaCallable f,
    const MetaFunctionParameters& args)
  {
    f(args);
  }

  void SignalSubscriber::call(const MetaFunctionParameters& args)
  {
    if (handler)
    {
      if (eventLoop)
      {
        // Event emission is always asynchronous
        MetaFunctionParameters copy = args.copy();
        eventLoop->asyncCall(0,
          boost::bind(&functor_call, handler, copy));
      }
      else
        handler(args);
    }
    if (target.isValid())
      target.metaEmit(method, args);
  }

  SignalBase::Link SignalBase::connect(GenericFunction callback, EventLoop* ctx)
  {
    return connect(SignalSubscriber(callback, ctx));
  }

  SignalBase::Link SignalBase::connect(qi::GenericObject o, unsigned int slot)
  {
    return connect(SignalSubscriber(o, slot));
  }

  SignalBase::Link SignalBase::connect(const SignalSubscriber& src)
  {
    boost::recursive_mutex::scoped_lock sl(_p->mutex);
    Link res = ++linkUid;
    _p->subscriberMap[res] = src;
    SignalSubscriber& s = _p->subscriberMap[res];
    s.linkId = res;
    s.source = this;
    if (s.target.isValid())
    {
      GenericObject o = s.target;
      Manageable* m = o.type->manageable(o.value);
      if (m)
        m->addRegistration(s);
    }
    return res;
  }

  SignalBase::SignalBase(const std::string& sig)
    : _p(new SignalBasePrivate)
  {
    _p->signature = sig;
  }

  bool SignalBasePrivate::disconnect(const SignalBase::Link& l)
  {
    boost::recursive_mutex::scoped_lock sl(mutex);
    SignalSubscriberMap::iterator it = subscriberMap.find(l);
    if (it == subscriberMap.end())
      return false;
    SignalSubscriber s = it->second;
    subscriberMap.erase(it);
    if (s.target.isValid())
    {
      Manageable* m = s.target.type->manageable(s.target.value);
      if (m)
        m->removeRegistration(l);
    }
    return true;
  }

  bool SignalBase::disconnect(const Link &link) {
    return _p->disconnect(link);
  }

  SignalBase::~SignalBase()
  {
    SignalSubscriberMap::iterator i;
    std::vector<Link> links;
    for (i = _p->subscriberMap.begin(); i!= _p->subscriberMap.end(); ++i)
    {
      links.push_back(i->first);
    }
    for (unsigned i=0; i<links.size(); ++i)
      disconnect(links[i]);
  }

  std::vector<SignalSubscriber> SignalBase::subscribers()
  {
    std::vector<SignalSubscriber> res;
    boost::recursive_mutex::scoped_lock sl(_p->mutex);
    SignalSubscriberMap::iterator i;
    for (i = _p->subscriberMap.begin(); i!= _p->subscriberMap.end(); ++i)
      res.push_back(i->second);
    return res;
  }

  void SignalBasePrivate::reset() {
    boost::recursive_mutex::scoped_lock sl(mutex);
    SignalSubscriberMap::iterator it = subscriberMap.begin();
    while (it != subscriberMap.end()) {
      disconnect(it->first);
      it = subscriberMap.begin();
    }
  }

}
