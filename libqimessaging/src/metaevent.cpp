/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include "src/metaevent_p.hpp"
#include <qimessaging/metaevent.hpp>
#include <qimessaging/object.hpp>
namespace qi {

  MetaEvent::MetaEvent()
    : _p(new MetaEventPrivate())
  {
  }

  MetaEvent::MetaEvent(const std::string &sig)
    : _p(new MetaEventPrivate(sig))
  {
  }

  MetaEvent::MetaEvent(const MetaEvent &other)
    : _p(new MetaEventPrivate())
  {
    *_p = *(other._p);
  }

  MetaEvent& MetaEvent::operator=(const MetaEvent &other)
  {
    *_p = *(other._p);
    return (*this);
  }

  MetaEvent::~MetaEvent()
  {
    delete _p;
  }

  const std::string &MetaEvent::signature() const
  {
    return _p->signature();
  }

  unsigned int       MetaEvent::index() const
  {
    return _p->index();
  }

  class DropResult: public FunctorResultBase
  {
  public:
    virtual void setValue(const qi::Buffer &buffer) {}
    virtual void setError(const std::string &sig, const qi::Buffer& msg)
    {
      qiLogError("object") << "Event handler returned an error";
    }
  };

  std::vector<MetaEvent::Subscriber>
  MetaEvent::subscribers() const
  {
    std::vector<MetaEvent::Subscriber> res;
    for ( MetaEventPrivate::Subscribers::iterator i = _p->_subscribers.begin();
      i != _p->_subscribers.end(); ++i)
    res.push_back(i->second);
    return res;
  }

  void MetaEvent::Subscriber::call(const FunctorParameters &args)
  {
    FunctorResult dummy(
      boost::shared_ptr<FunctorResultBase>(new DropResult()));
    if (handler)
      handler->call(args, dummy);
    if (target)
      target->metaCall(method, args, dummy);
  }
};
