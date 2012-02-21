/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef   	REMOTEOBJECT_P_HPP_
# define   	REMOTEOBJECT_P_HPP_

#include <qimessaging/datastream.hpp>
#include <qimessaging/object.hpp>
#include <string>

namespace qi {

  class TransportSocket;

  class RemoteObject : public qi::Object {
  public:
    explicit RemoteObject(qi::TransportSocket *ts, unsigned int service);

    virtual void metaCall(const std::string &method, const std::string &sig, DataStream &in, DataStream &out);

  protected:
    qi::TransportSocket *_ts;
    unsigned int         _service;
  };
}



#endif	    /* !REMOTEOBJECT_P_PP_ */
