#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QIMESSAGING_TRANSPORTSOCKET_HPP_
#define _QIMESSAGING_TRANSPORTSOCKET_HPP_

# include <qi/future.hpp>
# include <qimessaging/api.hpp>
# include <qimessaging/message.hpp>
# include <qimessaging/url.hpp>
# include <qi/eventloop.hpp>
# include <qitype/signal.hpp>
# include <string>
# include <boost/enable_shared_from_this.hpp>

#ifdef _MSC_VER
#  pragma warning( push )
#  pragma warning( disable: 4251 )
#endif

namespace qi
{
  class Session;
  class TransportSocketPrivate;
  class QIMESSAGING_API TransportSocket : public boost::enable_shared_from_this<TransportSocket>
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
    explicit TransportSocket(qi::EventLoop* eventLoop = qi::getDefaultNetworkEventLoop());
    virtual ~TransportSocket();

    virtual qi::FutureSync<bool> connect(const qi::Url &url) = 0;
    virtual qi::FutureSync<void> disconnect()                = 0;

    virtual bool send(const qi::Message &msg)                = 0;
    /// Must be called once if the socket is obtained through TransportServer::newConnection()
    virtual void  startReading() = 0;

    bool    isConnected() const;
    Status  status() const;
    int     error() const;
    qi::Url url() const;

    qi::SignalBase::Link messagePendingConnect(unsigned int serviceId, boost::function<void (const qi::Message&)> fun);
    bool                 messagePendingDisconnect(unsigned int serviceId, qi::SignalBase::Link linkId);

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

  QIMESSAGING_API TransportSocketPtr makeTransportSocket(const std::string &protocol, qi::EventLoop *eventLoop = getDefaultNetworkEventLoop());
}


QI_TYPE_NOT_CLONABLE(qi::TransportSocket);

#ifdef _MSC_VER
#  pragma warning( pop )
#endif

#endif  // _QIMESSAGING_TRANSPORTSOCKET_HPP_
