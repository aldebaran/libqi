/*
** Author(s):
**  - Herve Cuche <hcuche@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <iostream>
#include "src/transport_socket_p.hpp"

namespace qi
{
  TransportSocketPrivate::TransportSocketPrivate(TransportSocket *socket)
    : tcd(NULL)
    , connected(false)
    , readHdr(true)
    , msg(0)
    , self(socket)
  {
  }

  TransportSocketPrivate::~TransportSocketPrivate()
  {
  }

// if msecs < 0 no timeout
  bool TransportSocketPrivate::waitForConnected(int msecs)
  {
    // no timeout
    if (msecs < 0)
    {
      while (!isConnected())
        ;

      return true;
    }

    while (!isConnected() && msecs > 0)
    {
      qi::os::msleep(1);
      msecs--;
    }

    // timeout
    if (msecs == 0)
      return false;

    return true;
  }

  bool TransportSocketPrivate::waitForDisconnected(int msecs)
  {
    // no timeout
    if (msecs < 0)
    {
      while (isConnected())
        ;

      return true;
    }

    while (isConnected() && msecs > 0)
    {
      qi::os::msleep(1);
      msecs--;
    }

    // timeout
    if (msecs == 0)
      return false;

    return true;
  }

  bool TransportSocketPrivate::waitForId(int id, int msecs)
  {
    if (!isConnected())
    {
      qiLogError("qimessaging.TransportSocket") << "socket is not connected.";
      return false;
    }

    std::map<unsigned int, qi::Message*>::iterator it;
    {
      {
        boost::mutex::scoped_lock l(mtx);
        it = msgSend.find(id);
        if (it != msgSend.end())
          return true;
        if (!isConnected())
          return false;
        if (msecs > 0)
          cond.timed_wait(l, boost::posix_time::milliseconds(msecs));
        else
          cond.wait(l);
        it = msgSend.find(id);
        if (it != msgSend.end())
          return true;
      }
    }
    return false;
  }

  bool TransportSocketPrivate::read(int id, qi::Message *msg)
  {
    if (!isConnected())
    {
      qiLogError("qimessaging.TransportSocket") << "socket is not connected.";
      return false;
    }

    std::map<unsigned int, qi::Message*>::iterator it;
    {
      boost::mutex::scoped_lock l(mtx);
      it = msgSend.find(id);
      if (it != msgSend.end())
      {
        *msg = *it->second;
        delete it->second;
        msgSend.erase(it);
        return true;
      }
    }

    qiLogError("qimessaging.TransportSocket") << "message #" << id
                                              << " could not be found.";

    return false;
  }

  void TransportSocketPrivate::setCallbacks(TransportSocketInterface *delegate)
  {
    tcd = delegate;
  }

  bool TransportSocketPrivate::isConnected()
  {
    return connected;
  }
}
