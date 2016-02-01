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
# include <boost/asio/ssl.hpp>
# include <qi/api.hpp>
# include "message.hpp"
# include <qi/url.hpp>
# include "transportsocket.hpp"
# include <qi/eventloop.hpp>
# include "messagedispatcher.hpp"

namespace qi
{
  class TcpTransportSocket : public TransportSocket, public boost::enable_shared_from_this<TcpTransportSocket>
  {
  public:
    explicit TcpTransportSocket(EventLoop* eventloop = getEventLoop(), bool ssl = false, void* s = 0);
    virtual ~TcpTransportSocket();

    virtual qi::FutureSync<void> connect(const qi::Url &url);
    virtual qi::FutureSync<void> disconnect();
    virtual bool send(const qi::Message &msg);
    virtual void startReading();
    virtual qi::Url remoteEndpoint() const;
  private:
    using SocketPtr = boost::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>;
    void error(const std::string& erc);
    void onResolved(const boost::system::error_code& erc,
                    boost::asio::ip::tcp::resolver::iterator it,
                    qi::Promise<void> connectPromise);
    void onConnected(const boost::system::error_code& erc, SocketPtr s,
                    qi::Promise<void> connectPromise);
    void handshake(const boost::system::error_code& erc, SocketPtr s,
                    qi::Promise<void> connectPromise);
    void onReadHeader(const boost::system::error_code& erc, std::size_t, SocketPtr s);
    void onReadData(const boost::system::error_code& erc, std::size_t, SocketPtr s);
    void send_(qi::Message msg);
    void sendCont(const boost::system::error_code& erc, qi::Message msg, SocketPtr s);
    void setSocketOptions();
    void _continueReading();
    bool _ssl;
    bool _sslHandshake;
    boost::asio::ssl::context _sslContext;
   SocketPtr _socket;

    bool                _abort; // used to notify send callback sendCont that we are dead

    // data to rebuild message
    qi::Message         _msg;
    bool                _connecting;

    boost::mutex        _sendQueueMutex; // protects _sendQueue, _sending and closing
    std::deque<Message> _sendQueue;
    bool                _sending;
    mutable boost::recursive_mutex        _closingMutex;
    boost::shared_ptr<boost::asio::ip::tcp::resolver> _r;

  };

  using TcpTransportSocketPtr = boost::shared_ptr<TcpTransportSocket>;

}

#endif  // _SRC_TCPTRANSPORTSOCKET_HPP_
