#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_TRANSPORTSOCKET_HPP_
#define _SRC_TRANSPORTSOCKET_HPP_

# include <boost/noncopyable.hpp>
# include <boost/variant.hpp>
# include <qi/future.hpp>
# include "message.hpp"
# include <qi/url.hpp>
# include <qi/eventloop.hpp>
# include <qi/signal.hpp>
# include <qi/binarycodec.hpp>
# include <string>
# include "messagedispatcher.hpp"
# include "streamcontext.hpp"

namespace qi {
  namespace detail {
    template<>
    struct HasLess<boost::variant<std::string, qi::Message> >{
      static const bool value = false;
    };
  }
}

QI_TYPE_CONCRETE(boost::variant<std::string QI_COMMA qi::Message>);

namespace qi
{
  class Session;

  class TransportSocket : private boost::noncopyable, public StreamContext
  {
  public:
    virtual ~TransportSocket();
    enum Status {
      Status_Disconnected  = 0,
      Status_Connecting    = 1,
      Status_Connected     = 2,
      Status_Disconnecting = 3,
    };
    enum Event {
      Event_Error = 0,
      Event_Message = 1,
    };

    explicit TransportSocket(qi::EventLoop* eventLoop = qi::getEventLoop())
      : _eventLoop(NULL)
      , _err(0)
      , _status(Status_Disconnected)
    {
      connected.setCallType(MetaCallType_Queued);
      disconnected.setCallType(MetaCallType_Queued);
      // Set messageReady signal to async mode to protect our network thread
      messageReady.setCallType(MetaCallType_Queued);
      socketEvent.setCallType(MetaCallType_Queued);
    }

    virtual qi::FutureSync<void> connect(const qi::Url &url) = 0;
    virtual qi::FutureSync<void> disconnect()                = 0;

    virtual bool send(const qi::Message &msg)                = 0;
    /// Must be called once if the socket is obtained through TransportServer::newConnection()
    virtual void  startReading() = 0;

    virtual qi::Url remoteEndpoint() const = 0;

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

    qi::SignalLink messagePendingConnect(unsigned int serviceId, unsigned int objectId, boost::function<void (const qi::Message&)> fun) {
      return _dispatcher.messagePendingConnect(serviceId, objectId, fun);
    }

    bool                 messagePendingDisconnect(unsigned int serviceId, unsigned int objectId, qi::SignalLink linkId) {
      return _dispatcher.messagePendingDisconnect(serviceId, objectId, linkId);
    }

  protected:
    qi::EventLoop*          _eventLoop;
    qi::MessageDispatcher   _dispatcher;

    int                     _err;
    TransportSocket::Status _status;
    qi::Url                 _url;

  public:
    // C4251
    qi::Signal<>                   connected;
    // C4251
    qi::Signal<std::string>        disconnected;
    // C4251
    qi::Signal<const qi::Message&> messageReady;
    typedef boost::variant<std::string, qi::Message> SocketEventData;
    // C4251
    qi::Signal<SocketEventData>  socketEvent;
  };

  typedef boost::shared_ptr<TransportSocket> TransportSocketPtr;

  TransportSocketPtr makeTransportSocket(const std::string &protocol, qi::EventLoop *eventLoop = getEventLoop());

}

#endif  // _SRC_TRANSPORTSOCKET_HPP_
