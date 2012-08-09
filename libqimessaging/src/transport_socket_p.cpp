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
    : tcd()
    , connected(false)
    , status(0)
    , readHdr(true)
    , msg(0)
    , self(socket)
  {
  }

  TransportSocketPrivate::~TransportSocketPrivate()
  {
    delete msg;
  }

// if msecs < 0 no timeout
  bool TransportSocketPrivate::waitForConnected(int msecs)
  {
    // no timeout
    if (msecs < 0)
    {
      while (!isConnected() && status == 0)
        ;
      return true;
    }

    while ((!isConnected() && status == 0) && msecs > 0)
    {
      qi::os::msleep(1);
      msecs--;
    }
    return isConnected();
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

    return !isConnected();
  }

  bool TransportSocketPrivate::waitForId(int id, int msecs)
  {
    if (!isConnected())
    {
      qiLogError("qimessaging.TransportSocket") << "socket is not connected.";
      return false;
    }
    int wait = msecs;
    int64_t start = os::ustime();
    std::map<unsigned int, TransportSocketPrivate::PendingMessage>::iterator it;
    do
    {
      boost::mutex::scoped_lock l(mtx);
      it = msgSend.find(id);
      if (it != msgSend.end())
        return true;
      if (!isConnected())
        return false;
      if (msecs > 0)
        cond.timed_wait(l, boost::posix_time::milliseconds(wait));
      else
        cond.wait(l);
      it = msgSend.find(id);
      if (it != msgSend.end())
        return true;
      wait =  msecs - (os::ustime()-start) / 1000LL;
    }
    while (wait > 0);
    return false;
  }

  bool TransportSocketPrivate::read(int id, qi::Message *msg)
  {
    if (!isConnected())
    {
      qiLogError("qimessaging.TransportSocket") << "socket is not connected.";
      return false;
    }

    std::map<unsigned int, TransportSocketPrivate::PendingMessage>::iterator it;
    {
      boost::mutex::scoped_lock l(mtx);
      it = msgSend.find(id);
      if (it != msgSend.end())
      {
        *msg = *(it->second.msg);
        delete it->second.msg;
        msgSend.erase(it);
        return true;
      }
    }

    qiLogError("qimessaging.TransportSocket") << "message #" << id
                                              << " could not be found.";

    return false;
  }

  void TransportSocketPrivate::addCallbacks(TransportSocketInterface *delegate)
  {
    if (delegate)
    {
      boost::recursive_mutex::scoped_lock l(mtxCallback);
      tcd.push_back(delegate);
    }
    else
      qiLogError("") << "Trying to set invalid callback on TransportSoket.";
  }

  void TransportSocketPrivate::removeCallbacks(TransportSocketInterface *delegate)
  {
    if (delegate)
    {
      boost::recursive_mutex::scoped_lock l(mtxCallback);
      std::vector<TransportSocketInterface *>::iterator it;
      for (it = tcd.begin(); it != tcd.end(); ++it)
      {
        if (*it == delegate)
        {
          tcd.erase(it);
          break;
        }
      }
    }
    else
      qiLogError("") << "Trying to erase invalid callback on TransportSoket.";
  }

  bool TransportSocketPrivate::isConnected() const
  {
    return connected;
  }
}
