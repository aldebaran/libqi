#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_BOUNDOBJECT_HPP_
#define _SRC_BOUNDOBJECT_HPP_

#include <string>
#include <boost/thread/mutex.hpp>
#include <boost/signals2.hpp>
#include <qimessaging/api.hpp>
#include <qimessaging/session.hpp>
#include "transportserver.hpp"
#include <qi/atomic.hpp>

#include "objecthost.hpp"

namespace qi {

  class GenericObject;
  class ServiceDirectoryClient;

  // (service, linkId)
  struct RemoteSignalLink
  {
    RemoteSignalLink()
      : localSignalLinkId(0)
      , event(0)
    {}
    RemoteSignalLink(SignalLink localSignalLinkId, unsigned int event)
    : localSignalLinkId(localSignalLinkId)
    , event(event) {}
    SignalLink localSignalLinkId;
    unsigned int event;
  };


  class BoundObject {
  public:
    //Server Interface
    virtual ~BoundObject() {}
    virtual void onMessage(const qi::Message &msg, TransportSocketPtr socket) = 0;
    virtual void onSocketDisconnected(qi::TransportSocketPtr socket, std::string error) = 0;
  };

  //Bound Object, represent an object bound on a server
  // this is not an object..
  class ServiceBoundObject : public BoundObject, public ObjectHost, boost::noncopyable {
  public:
    ServiceBoundObject(unsigned int serviceId, unsigned int objectId,
                       qi::AnyObject obj,
                       qi::MetaCallType mct = qi::MetaCallType_Queued,
                       bool bindTerminate = false,
                       ObjectHost* owner = 0);
    virtual ~ServiceBoundObject();

  public:
    //PUBLIC BOUND METHODS
    SignalLink           registerEvent(unsigned int serviceId, unsigned int eventId, SignalLink linkId);
    void           unregisterEvent(unsigned int serviceId, unsigned int eventId, SignalLink linkId);
    qi::MetaObject metaObject(unsigned int serviceId);
    void           terminate(unsigned int serviceId); //bound only in special cases
    qi::AnyValue   property(const AnyValue& name);
    void           setProperty(const AnyValue& name, AnyValue value);
    std::vector<std::string> properties();
  public:
    inline qi::TransportSocketPtr currentSocket() const {
#ifndef NDEBUG
      if (_callType != MetaCallType_Direct)
        qiLogWarning("qimessaging.boundobject") << " currentSocket() used but callType is not direct";
#endif
      return _currentSocket;
    }

    inline AnyObject object() { return _object;}
  public:
    //BoundObject Interface
    virtual void onMessage(const qi::Message &msg, TransportSocketPtr socket);
    virtual void onSocketDisconnected(qi::TransportSocketPtr socket, std::string error);

    qi::Signal<ServiceBoundObject*> onDestroy;
  private:
    qi::AnyObject createServiceBoundObjectType(ServiceBoundObject *self, bool bindTerminate = false);

  private:
    // remote link id -> local link id
    typedef std::map<SignalLink, RemoteSignalLink>             ServiceSignalLinks;
    typedef std::map<qi::TransportSocketPtr, ServiceSignalLinks> BySocketServiceSignalLinks;

    //Event handling (no lock needed)
    BySocketServiceSignalLinks  _links;

  private:
    qi::TransportSocketPtr _currentSocket;
    unsigned int           _serviceId;
    unsigned int           _objectId;
    qi::AnyObject          _object;
    qi::AnyObject          _self;
    qi::MetaCallType       _callType;
    qi::ObjectHost*        _owner;
    boost::mutex           _mutex; // prevent parallel onMessage on self execution
    friend class ::qi::ObjectHost;
  };


  typedef boost::shared_ptr<BoundObject> BoundAnyObject;

  qi::BoundAnyObject makeServiceBoundAnyObject(unsigned int serviceId, qi::AnyObject object, qi::MetaCallType mct = qi::MetaCallType_Auto);

}

#endif  // _SRC_BOUNDOBJECT_HPP_
