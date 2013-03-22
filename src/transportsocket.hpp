#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QIMESSAGING_TRANSPORTSOCKET_HPP_
#define _QIMESSAGING_TRANSPORTSOCKET_HPP_

# include <qi/future.hpp>
# include <qimessaging/api.hpp>
# include "message.hpp"
# include <qimessaging/url.hpp>
# include <qi/eventloop.hpp>
# include <qitype/signal.hpp>
# include <string>
# include <boost/enable_shared_from_this.hpp>
# include "messagedispatcher.hpp"

namespace qi
{
  class Session;
  class TransportSocketPrivate;
  class TransportSocket : public boost::enable_shared_from_this<TransportSocket>
  {
    QI_DISALLOW_COPY_AND_ASSIGN(TransportSocket);

  public:
    enum Status {
      Status_Disconnected  = 0,
      Status_Connecting    = 1,
      Status_Connected     = 2,
      Status_Disconnecting = 3,
    };

  public:
    explicit TransportSocket(qi::EventLoop* eventLoop = qi::getDefaultNetworkEventLoop())
    {
      // Set messageReady signal to async mode to protect our network thread
      messageReady.setCallType(MetaCallType_Queued);
    }

    virtual qi::FutureSync<void> connect(const qi::Url &url) = 0;
    virtual qi::FutureSync<void> disconnect()                = 0;

    virtual bool send(const qi::Message &msg)                = 0;
    /// Must be called once if the socket is obtained through TransportServer::newConnection()
    virtual void  startReading() = 0;

    qi::Url url() const {
      return _url;
    }

    Status status() const {
      return _status;
    }

    int error() const
    {
      return _err;
    }

    bool isConnected() const
    {
      return _status == qi::TransportSocket::Status_Connected;
    }

    static const unsigned int ALL_OBJECTS = (unsigned int)-1;

    qi::SignalBase::Link messagePendingConnect(unsigned int serviceId, unsigned int objectId, boost::function<void (const qi::Message&)> fun) {
      return _dispatcher.messagePendingConnect(serviceId, objectId, fun);
    }

    bool                 messagePendingDisconnect(unsigned int serviceId, unsigned int objectId, qi::SignalBase::Link linkId) {
      return _dispatcher.messagePendingDisconnect(serviceId, objectId, linkId);
    }

  public:
    qi::EventLoop*          _eventLoop;
    qi::MessageDispatcher   _dispatcher;

    int                     _err;
    TransportSocket::Status _status;
    qi::Url                 _url;

  public:
    // C4251
    qi::Signal<void ()>                   connected;
    // C4251
    qi::Signal<void (int error)>          disconnected;
    // C4251
    qi::Signal<void (const qi::Message&)> messageReady;

  protected:
    explicit TransportSocket(TransportSocketPrivate *p);
    TransportSocketPrivate *_p;
  };

  typedef boost::shared_ptr<TransportSocket> TransportSocketPtr;

  TransportSocketPtr makeTransportSocket(const std::string &protocol, qi::EventLoop *eventLoop = getDefaultNetworkEventLoop());
}


QI_TYPE_NOT_CLONABLE(qi::TransportSocket);


#endif  // _QIMESSAGING_TRANSPORTSOCKET_HPP_
