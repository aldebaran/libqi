#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_BOUNDOBJECT_HPP_
#define _SRC_BOUNDOBJECT_HPP_

#include <string>
#include <set>
#include <boost/thread/recursive_mutex.hpp>
#include <qimessaging/api.hpp>
#include <qimessaging/session.hpp>
#include <qimessaging/transportserver.hpp>
#include <qi/atomic.hpp>

namespace qi {

  class GenericObject;
  class ServiceDirectoryClient;

  // (service, linkId)
  struct RemoteLink
  {
    RemoteLink()
      : localLinkId(0)
      , event(0)
    {}
    RemoteLink(unsigned int localLinkId, unsigned int event)
    : localLinkId(localLinkId)
    , event(event) {}
    unsigned int localLinkId;
    unsigned int event;
  };


  class BoundObject {
  public:
    //Server Interface
    virtual void onMessage(const qi::Message &msg, TransportSocketPtr socket) = 0;
    virtual void onSocketDisconnected(qi::TransportSocketPtr socket, int error) = 0;
  };

  //Bound Object, represent an object bound on a server
  // this is not an object..
  class ServiceBoundObject : public BoundObject {
  public:
    ServiceBoundObject(unsigned int serviceId, qi::ObjectPtr obj, qi::MetaCallType mct = qi::MetaCallType_Auto);
    virtual ~ServiceBoundObject();

  public:
    //PUBLIC BOUND METHODS
    unsigned int   registerEvent(unsigned int serviceId, unsigned int eventId, unsigned int linkId);
    void           unregisterEvent(unsigned int serviceId, unsigned int eventId, unsigned int linkId);
    qi::MetaObject metaObject(unsigned int serviceId);

  public:
    inline qi::TransportSocketPtr currentSocket() const { return _currentSocket; }

  public:
    //BoundObject Interface
    virtual void onMessage(const qi::Message &msg, TransportSocketPtr socket);
    virtual void onSocketDisconnected(qi::TransportSocketPtr socket, int error);

  private:
    qi::ObjectPtr createServiceBoundObjectType(ServiceBoundObject *self);

  private:
    // remote link id -> local link id
    typedef std::map<unsigned int, RemoteLink>             ServiceLinks;
    typedef std::map<qi::TransportSocketPtr, ServiceLinks> BySocketServiceLinks;

    //Event handling (no lock needed)
    BySocketServiceLinks  _links;

  private:
    qi::TransportSocketPtr _currentSocket;
    unsigned int           _serviceId;
    qi::ObjectPtr          _object;
    qi::ObjectPtr          _self;
    qi::MetaCallType       _callType;
  };


  typedef boost::shared_ptr<BoundObject> BoundObjectPtr;

  qi::BoundObjectPtr makeServiceBoundObjectPtr(unsigned int serviceId, qi::ObjectPtr object, qi::MetaCallType mct = qi::MetaCallType_Auto);

}

QI_TYPE_NOT_CONSTRUCTIBLE(qi::ServiceBoundObject);


#endif  // _SRC_SESSIONSERVER_HPP_
