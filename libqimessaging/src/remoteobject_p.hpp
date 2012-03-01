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
#include <string>

namespace qi {

  class TransportSocket;

  class RemoteObject : public qi::Object {
  public:
    explicit RemoteObject(qi::TransportSocket *ts, unsigned int service, qi::MetaObject *mo);
    ~RemoteObject();

    virtual void metaCall(unsigned int method, const std::string &sig, FunctorParameters &in, FunctorResult &out);

  protected:
    qi::TransportSocket *_ts;
    unsigned int         _service;
  };
}



#endif  // _SRC_REMOTEOBJECT_P_HPP_
