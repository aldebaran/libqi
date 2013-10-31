#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_TCPTRANSPORTSOCKET_HPP_
#define _SRC_TCPTRANSPORTSOCKET_HPP_


# include <string>
# include <queue>
# include <boost/thread/recursive_mutex.hpp>
# include <boost/asio.hpp>
# ifdef WITH_SSL
# include <boost/asio/ssl.hpp>
# endif
# include <qimessaging/api.hpp>
# include "message.hpp"
# include <qimessaging/url.hpp>
# include "transportsocket.hpp"
# include <qi/eventloop.hpp>
# include "transportsocket.hpp"
# include <qi/eventloop.hpp>
# include "messagedispatcher.hpp"

namespace qi
{
  class TcpTransportSocket : public TransportSocket, public boost::enable_shared_from_this<TcpTransportSocket>
  {
  public:
    explicit TcpTransportSocket(EventLoop* eventloop = getDefaultNetworkEventLoop(), bool ssl = false, void* s = 0);
    virtual ~TcpTransportSocket();

    virtual qi::FutureSync<void> connect(const qi::Url &url);
    virtual qi::FutureSync<void> disconnect();
    virtual bool send(const qi::Message &msg);
    virtual void startReading();

  private:
#ifdef WITH_SSL
    typedef boost::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket> > SocketPtr;
#else
    typedef boost::shared_ptr<boost::asio::ip::tcp::socket> SocketPtr;
#endif
    void error(const boost::system::error_code& erc);
    void onConnected(const boost::system::error_code& erc, SocketPtr s);
    void handshake(const boost::system::error_code& erc, SocketPtr s);
    void onReadHeader(const boost::system::error_code& erc, std::size_t, SocketPtr s);
    void onReadData(const boost::system::error_code& erc, std::size_t, SocketPtr s);
    void send_(qi::Message msg);
    void sendCont(const boost::system::error_code& erc, qi::Message msg, SocketPtr s);
    void setSocketOptions();
    bool _ssl;
    bool _sslHandshake;
#ifdef WITH_SSL
    boost::asio::ssl::context _sslContext;
#endif
   SocketPtr _socket;

    bool                _abort; // used to notify send callback sendCont that we are dead
    qi::Promise<void>   _connectPromise;
    qi::Promise<void>   _disconnectPromise;

    // data to rebuild message
    qi::Message        *_msg;
    bool                _connecting;

    boost::mutex        _sendQueueMutex; // protects _sendQueue, _sending and closing
    std::deque<Message> _sendQueue;
    bool                _sending;
    boost::recursive_mutex        _closingMutex;
  };

  typedef boost::shared_ptr<TcpTransportSocket> TcpTransportSocketPtr;

}

#endif  // _SRC_TCPTRANSPORTSOCKET_HPP_
