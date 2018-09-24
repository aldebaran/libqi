#pragma once
#ifndef _SRC_MESSAGESOCKET_HPP_
#define _SRC_MESSAGESOCKET_HPP_

# include <boost/asio/io_service.hpp>
# include <boost/noncopyable.hpp>
# include <boost/variant.hpp>
# include <boost/optional.hpp>
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
  inline boost::asio::io_service* asIoServicePtr(EventLoop* e)
  {
    return static_cast<boost::asio::io_service*>(e->nativeHandle());
  }

  class Session;

  class MessageSocket : private boost::noncopyable, public StreamContext
  {
  public:
    virtual ~MessageSocket();

    // Do not change the values because TcpMessageSocket::status() relies on it.
    enum class Status {
      Disconnected  = 0,
      Connecting    = 1,
      Connected     = 2,
      Disconnecting = 3,
    };
    enum Event {
      Event_Error = 0,
      Event_Message = 1,
    };

    explicit MessageSocket(qi::EventLoop* eventLoop = qi::getNetworkEventLoop())
      : _eventLoop(eventLoop)
      , _dispatcher{ &_signalsStrand }
      // connected is the only signal to be synchronous, because it will always be the first signal
      // emitted (so no other asynchronous signal emission will overlap with it) and it's not
      // emitted from the network event loop worker
      , disconnected{ &_signalsStrand }
      , messageReady{ &_signalsStrand }
      , socketEvent{ &_signalsStrand }
    {
      connected.setCallType(MetaCallType_Direct);
      disconnected.setCallType(MetaCallType_Direct);
      messageReady.setCallType(MetaCallType_Direct);
      socketEvent.setCallType(MetaCallType_Direct);
    }

    virtual qi::FutureSync<void> connect(const qi::Url &url) = 0;
    virtual qi::FutureSync<void> disconnect()                = 0;

    virtual bool send(qi::Message msg) = 0;

    /// Start reading if is not already reading.
    /// Must be called once if the socket is obtained through TransportServer::newConnection()
    virtual bool  ensureReading() = 0;

    virtual Status status() const = 0;
    virtual boost::optional<qi::Url> remoteEndpoint() const = 0;

    virtual qi::Url url() const = 0;

    bool isConnected() const;

    static const unsigned int ALL_OBJECTS = (unsigned int)-1;

    qi::SignalLink messagePendingConnect(unsigned int serviceId, unsigned int objectId, boost::function<void (const qi::Message&)> fun) {
      return _dispatcher.messagePendingConnect(serviceId, objectId, fun);
    }

    void messagePendingDisconnect(unsigned int serviceId, unsigned int objectId, qi::SignalLink linkId) {
      _dispatcher.messagePendingDisconnect(serviceId, objectId, linkId);
    }

  protected:
    qi::EventLoop* _eventLoop;
    Strand _signalsStrand; // Must be declared before the MessageDispatcher and the signals.
    qi::MessageDispatcher _dispatcher;

  public:
    // C4251
    qi::Signal<>                   connected;
    // C4251
    qi::Signal<std::string>        disconnected;
    // C4251
    qi::Signal<const qi::Message&> messageReady;
    using SocketEventData = boost::variant<std::string, qi::Message>;
    // C4251
    qi::Signal<SocketEventData>  socketEvent;
  };

  using MessageSocketPtr = boost::shared_ptr<MessageSocket>;
  MessageSocketPtr makeMessageSocket(const std::string &protocol, qi::EventLoop *eventLoop = getNetworkEventLoop());
}

#endif  // _SRC_MESSAGESOCKET_HPP_
