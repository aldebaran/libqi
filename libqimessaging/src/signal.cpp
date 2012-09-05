/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <map>
#include <qi/atomic.hpp>

#include <boost/thread/recursive_mutex.hpp>

#include <qimessaging/signal.hpp>
#include <qimessaging/metavalue.hpp>

#include "src/object_p.hpp"

namespace qi {


  typedef std::map<SignalBase::Link, SignalSubscriber> SignalSubscriberMap;

  class SignalBasePrivate
  {
  public:
    SignalSubscriberMap              subscriberMap;
    std::string                signature;
    boost::recursive_mutex     mutex;
  };

  static qi::atomic<long> linkUid = 1;

  void SignalBase::operator()(
      qi::AutoMetaValue p1,
      qi::AutoMetaValue p2,
      qi::AutoMetaValue p3,
      qi::AutoMetaValue p4,
      qi::AutoMetaValue p5,
      qi::AutoMetaValue p6,
      qi::AutoMetaValue p7,
      qi::AutoMetaValue p8)
  {
    qi::AutoMetaValue* vals[8]= {&p1, &p2, &p3, &p4, &p5, &p6, &p7, &p8};
    std::vector<qi::MetaValue> params;
    for (unsigned i=0; i<8; ++i)
      if (vals[i]->value)
        params.push_back(*vals[i]);
    // Signature construction
    std::string signature;
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

  static void functor_call(MetaFunction f,
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
    if (target)
      target->metaEmit(method, args);
  }

  SignalBase::Link
  SignalBase::connect(MetaFunction callback, EventLoop* ctx)
  {
    return connect(SignalSubscriber(callback, ctx));
  }
  SignalBase::Link
  SignalBase::connect(qi::Object* o, unsigned int slot)
  {
    return connect(SignalSubscriber(o, slot));
  }
  SignalBase::Link
  SignalBase::connect(const SignalSubscriber& src)
  {
    boost::recursive_mutex::scoped_lock sl(_p->mutex);
    Link res = ++linkUid;
    _p->subscriberMap[res] = src;
    SignalSubscriber& s = _p->subscriberMap[res];
    s.linkId = res;
    s.source = this;
    if (s.target)
    {
      Object* o = s.target;
      boost::recursive_mutex::scoped_lock sl(o->_p->_mutexRegistration);
      o->_p->_registrations.push_back(s);
    }
    return res;
  }

  SignalBase::SignalBase(const std::string& sig)
  {
    _p = new SignalBasePrivate;
    _p->signature = sig;
  }

  bool SignalBase::disconnect(const Link& l)
  {
    boost::recursive_mutex::scoped_lock sl(_p->mutex);
    SignalSubscriberMap::iterator i = _p->subscriberMap.find(l);
    if (i == _p->subscriberMap.end())
      return false;
    SignalSubscriber s = i->second;
    _p->subscriberMap.erase(i);
    if (s.target)
    {
      boost::recursive_mutex::scoped_lock sl(s.target->_p->_mutexRegistration);
      std::vector<SignalSubscriber>& regs = s.target->_p->_registrations;
      // Look it up in vector, then swap with last.
      for (unsigned int i=0; i< regs.size(); ++i)
        if (s.linkId == regs[i].linkId)
        {
          regs[i] = regs[regs.size()-1];
          regs.pop_back();
          break;
        }
    }
    return true;
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
    delete _p;
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
}
