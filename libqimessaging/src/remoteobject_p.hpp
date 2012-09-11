/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#pragma once
#ifndef _SRC_REMOTEOBJECT_P_HPP_
#define _SRC_REMOTEOBJECT_P_HPP_

#include <qimessaging/datastream.hpp>
#include <qimessaging/transport_socket.hpp>
#include <qimessaging/object.hpp>
#include <boost/thread/mutex.hpp>
#include <string>
#include "src/object_p.hpp"

namespace qi {

  class TransportSocket;

  class RemoteObjectPrivate : public qi::ObjectPrivate, public qi::TransportSocketInterface {
  public:
    RemoteObjectPrivate(qi::TransportSocket *ts, unsigned int service, qi::MetaObject mo);
    ~RemoteObjectPrivate();

    void close();

  protected:

    virtual void onSocketReadyRead(TransportSocket *client, int id, void*);
    virtual void onSocketTimeout(TransportSocket *client, int id, void*);

    virtual void metaEmit(unsigned int event, const MetaFunctionParameters& args);
    virtual qi::Future<MetaFunctionResult> metaCall(unsigned int method, const MetaFunctionParameters& args, qi::Object::MetaCallType callType = qi::Object::MetaCallType_Auto);

    virtual unsigned int connect(unsigned int event, const SignalSubscriber& sub);
    virtual bool disconnect(unsigned int linkId);

  public:
    qi::TransportSocket                            *_ts;

  protected:
    unsigned int                                    _service;
    std::map<int, qi::Promise<MetaFunctionResult> > _promises;
    boost::mutex                                    _mutex;
  };


  class RemoteObject : public qi::Object {
  public:
    RemoteObject();
    RemoteObject(qi::TransportSocket *ts, unsigned int service, qi::MetaObject mo);
    ~RemoteObject();

    void close();
  };

}



#endif  // _SRC_REMOTEOBJECT_P_HPP_
