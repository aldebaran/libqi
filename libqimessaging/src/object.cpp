/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <iostream>
#include "src/object_p.hpp"

namespace qi {

  ObjectInterface::~ObjectInterface() {
  }

  Object::Object(qi::MetaObject metaObject)
    : _p(new ObjectPrivate(metaObject))
  {
  }

  Object::Object(qi::ObjectPrivate *pimpl)
    : _p(pimpl)
  {
  }

  Object::Object()
    : _p()
  {
  }

  Object::~Object() {
  }

  bool Object::isValid() const {
    return !!_p;
  }

  void Object::addCallbacks(ObjectInterface *callbacks, void *data)
  {
    if (!_p) {
      qiLogWarning("qi.object") << "Operating on invalid Object..";
      return;
    }
    return _p->addCallbacks(callbacks, data);
  }

  void Object::removeCallbacks(ObjectInterface *callbacks)
  {
    if (!_p) {
      qiLogWarning("qi.object") << "Operating on invalid Object..";
      return;
    }
    return _p->removeCallbacks(callbacks);
  }

  MetaObject &Object::metaObject() {
    if (!_p) {
      static qi::MetaObject fail;
      qiLogWarning("qi.object") << "Operating on invalid Object..";
      return fail;
    }
    return _p->metaObject();
  }

  qi::Future<MetaFunctionResult>
  Object::metaCall(unsigned int method, const MetaFunctionParameters& params, MetaCallType callType)
  {
    qi::Promise<MetaFunctionResult> out;
    if (!_p) {
      qiLogWarning("qi.object") << "Operating on invalid Object..";
      out.setError("Invalid object");
      return out.future();
    }
    return _p->metaCall(method, params, callType);
  }

  void Object::metaEmit(unsigned int event, const MetaFunctionParameters& args)
  {
    trigger(event, args);
  }

  void Object::trigger(unsigned int event, const MetaFunctionParameters &args)
  {
    if (!_p) {
      qiLogWarning("qi.object") << "Operating on invalid Object..";
      return;
    }
    return _p->trigger(event, args);
  }

  qi::Future<MetaFunctionResult>
  Object::xMetaCall(const std::string &retsig, const std::string &signature, const MetaFunctionParameters& args)
  {
    qi::Promise<MetaFunctionResult> out;
    if (!_p) {
      qiLogWarning("qi.object") << "Operating on invalid Object..";
      out.setError("Invalid object");
      return out.future();
    }
    return _p->xMetaCall(retsig, signature, args);
  }
  /// Resolve signature and bounce
  bool Object::xMetaEmit(const std::string &signature, const MetaFunctionParameters &in) {
    if (!_p) {
      qiLogWarning("qi.object") << "Operating on invalid Object..";
      return false;
    }
    return _p->xMetaEmit(signature, in);
  }

  /// Resolve signature and bounce
  unsigned int Object::xConnect(const std::string &signature, MetaFunction functor, EventLoop* ctx)
  {
    if (!_p) {
      qiLogWarning("qi.object") << "Operating on invalid Object..";
      return -1;
    }
    return _p->xConnect(signature, functor, ctx);
  }

  unsigned int Object::connect(unsigned int event, MetaFunction functor, EventLoop* ctx)
  {
    if (!_p) {
      qiLogWarning("qi.object") << "Operating on invalid Object..";
      return -1;
    }
    return connect(event, SignalSubscriber(functor, ctx));
  }

  unsigned int Object::connect(unsigned int event, const SignalSubscriber& sub)
  {
    if (!_p) {
      qiLogWarning("qi.object") << "Operating on invalid Object..";
      return -1;
    }
    return _p->connect(event, sub);
  }

  bool Object::disconnect(unsigned int linkId)
  {
    if (!_p) {
      qiLogWarning("qi.object") << "Operating on invalid Object..";
      return false;
    }
    return _p->disconnect(linkId);
  }

  unsigned int Object::connect(unsigned int signal, qi::Object target, unsigned int slot)
  {
    if (!_p) {
      qiLogWarning("qi.object") << "Operating on invalid Object..";
      return -1;
    }
    return _p->connect(signal, target, slot);
  }

  EventLoop* Object::eventLoop()
  {
    if (!_p) {
      qiLogWarning("qi.object") << "Operating on invalid Object..";
      return 0;
    }
    return _p->eventLoop();
  }

  void Object::moveToEventLoop(EventLoop* ctx)
  {
    if (!_p) {
      qiLogWarning("qi.object") << "Operating on invalid Object..";
      return;
    }
    _p->moveToEventLoop(ctx);
  }

  std::vector<SignalSubscriber> Object::subscribers(int eventId) const
  {
    std::vector<SignalSubscriber> res;
    if (!_p) {
      qiLogWarning("qi.object") << "Operating on invalid Object..";
      return res;
    }
    return _p->subscribers(eventId);
  }

  void Object::emitEvent(const std::string& eventName,
                         qi::AutoValue p1,
                         qi::AutoValue p2,
                         qi::AutoValue p3,
                         qi::AutoValue p4,
                         qi::AutoValue p5,
                         qi::AutoValue p6,
                         qi::AutoValue p7,
                         qi::AutoValue p8)
  {
    if (!_p) {
      qiLogWarning("qi.object") << "Operating on invalid Object..";
      return;
    }
    return _p->emitEvent(eventName, p1, p2, p3, p4, p5, p6, p7, p8);
  }

}

