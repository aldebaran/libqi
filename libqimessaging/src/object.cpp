/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <iostream>
#include <qi/application.hpp>
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
                         qi::AutoMetaValue p1,
                         qi::AutoMetaValue p2,
                         qi::AutoMetaValue p3,
                         qi::AutoMetaValue p4,
                         qi::AutoMetaValue p5,
                         qi::AutoMetaValue p6,
                         qi::AutoMetaValue p7,
                         qi::AutoMetaValue p8)
  {
    if (!_p) {
      qiLogWarning("qi.object") << "Operating on invalid Object..";
      return;
    }
    return _p->emitEvent(eventName, p1, p2, p3, p4, p5, p6, p7, p8);
  }

  // Factory system
  // We need thread-safeness, and we can be used at static init.
  // But at static init, thread-safeness is not required.
  // So lazy-init of the mutex should do the trick.
  static boost::recursive_mutex *_f_mutex_struct = 0;
  static boost::recursive_mutex *_f_mutex_load = 0;
  static std::vector<std::string>* _f_keys = 0;
  typedef std::map<std::string, boost::function<qi::Object (const std::string&)> > FactoryMap;
  static FactoryMap* _f_map = 0;
  static void _f_init()
  {
    if (!_f_mutex_struct)
    {
      _f_mutex_struct = new boost::recursive_mutex;
      _f_mutex_load = new boost::recursive_mutex;
      _f_keys = new std::vector<std::string>;
      _f_map = new FactoryMap;
    }
  }

  bool registerObjectFactory(const std::string& name, boost::function<qi::Object (const std::string&)> factory)
  {
    qiLogDebug("qi.factory") << "registering " << name;
    _f_init();
    boost::recursive_mutex::scoped_lock sl(*_f_mutex_struct);
    FactoryMap::iterator i = _f_map->find(name);
    if (i != _f_map->end())
      qiLogWarning("qi.object") << "Overriding factory for " <<name;
    else
      _f_keys->push_back(name);
    (*_f_map)[name] = factory;
    return true;
  }

  Object createObject(const std::string& name)
  {
    _f_init();
    boost::recursive_mutex::scoped_lock sl(*_f_mutex_struct);
    FactoryMap::iterator i = _f_map->find(name);
    if (i == _f_map->end())
      return Object();
    return (i->second)(name);
  }

  std::vector<std::string> listObjectFactories()
  {
    _f_init();
    boost::recursive_mutex::scoped_lock sl(*_f_mutex_struct);
    return *_f_keys;
  }

  std::vector<std::string> loadObject(const std::string& name, int flags)
  {
    /* Do not hold mutex_struct while calling dlopen/loadModule,
    * just in case static initialization of the module happens
    * in an other thread: it will likely call registerObjectFactory
    * which will acquire the mutex_struct.
    * We are still asserting that said initialization synchronously
    * finishes before dlopen/loadModule returns.
    */
    _f_init();
    std::vector<std::string>& keys = *_f_keys;
    boost::recursive_mutex::scoped_lock sl(*_f_mutex_load);
    unsigned int count = keys.size();
    qiLogDebug("qi.factory") << count <<" object before load";
    Application::loadModule(name, flags);
    boost::recursive_mutex::scoped_lock sl2(*_f_mutex_struct);
    qiLogDebug("qi.factory") << keys.size() <<" object after load";
    if (count != keys.size())
      return std::vector<std::string>(&keys[count], &keys[keys.size()]);
    else
      return std::vector<std::string>();
  }


}

