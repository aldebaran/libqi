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
#include <qimessaging/object.hpp>
#include <qimessaging/transport_socket.hpp>
#include <boost/thread/mutex.hpp>
#include <string>

namespace qi {

  class TransportSocket;

  class RemoteObject : public qi::Object, public qi::TransportSocketInterface {
  public:
    explicit RemoteObject(qi::TransportSocket *ts, unsigned int service, qi::MetaObject *mo);
    ~RemoteObject();

    virtual void onSocketReadyRead(TransportSocket *client, int id);
    virtual void metaCall(unsigned int method, const std::string &sig, qi::FunctorParameters &in, qi::FunctorResultPromiseBase *out);

  protected:
    qi::TransportSocket                           *_ts;
    unsigned int                                   _service;
    std::map<int, qi::FunctorResultPromiseBase *>  _promises;
    boost::mutex                                   _mutex;
  };

}



#endif  // _SRC_REMOTEOBJECT_P_HPP_
