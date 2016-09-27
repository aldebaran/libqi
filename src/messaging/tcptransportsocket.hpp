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
# include <boost/optional.hpp>
# include <qi/api.hpp>
# include "message.hpp"
# include <qi/url.hpp>
# include "transportsocket.hpp"
# include <qi/eventloop.hpp>
# include "messagedispatcher.hpp"

namespace qi
{
  /**
   * Socket handling the Qi messaging protocol.
   */
  // TODO: make this API more symmetric by clarifying the server and client roles.
  class TcpTransportSocket : public TransportSocket, public boost::enable_shared_from_this<TcpTransportSocket>
  {
  public:
    using Socket = boost::asio::ssl::stream<boost::asio::ip::tcp::socket>;
    using SocketPtr = boost::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>;

    /// @param s if not null, must be a connected socket on the server side
    explicit TcpTransportSocket(EventLoop* eventloop = getEventLoop(), bool ssl = false, SocketPtr s = {});
    virtual ~TcpTransportSocket();

    /// Connects the socket as a client and ensures that it is reading.
    virtual FutureSync<void> connect(const qi::Url &url);

    /// Stops reading the socket and disconnects it.
    virtual FutureSync<void> disconnect();

    virtual bool send(const qi::Message &msg);
    virtual void ensureReading();
    virtual qi::Url remoteEndpoint() const;
  private:
    /// Internal version of ensureReading taking a promise representing the
    /// status of the connection attempt.
    void ensureReading(qi::Promise<void> connectionAttemptPromise);

    /// Forces an error to be processed by the socket, leading to its disconnection.
    void error(const std::string& erc);

    void onResolved(const boost::system::error_code& erc,
                    boost::asio::ip::tcp::resolver::iterator it,
                    SocketPtr,
                    qi::Promise<void> connectPromise);
    void onConnected(const boost::system::error_code& erc, SocketPtr s,
                    qi::Promise<void> connectPromise);
    void handshake(const boost::system::error_code& erc, SocketPtr s,
                    qi::Promise<void> connectPromise);
    void onReadHeader(const boost::system::error_code& erc, std::size_t, SocketPtr s);
    void onReadData(const boost::system::error_code& erc, std::size_t, SocketPtr s);

    /// The effective sending of a message. Requires prior locking and assumes
    /// socket is present and connected.
    void send_(qi::Message msg);

    void sendCont(const boost::system::error_code& erc, qi::Message msg, SocketPtr s);

    /// Prepare the socket. Callers must lock _mutex first.
    void setSocketOptions();

    void _continueReading(qi::Promise<void> connectionAttemptPromise);
    bool _ssl;

    /// Type of the next SSL handshake to perform.
    /// No handshake must be performed if this value is not set.
    boost::optional<boost::asio::ssl::stream_base::handshake_type> _nextHandshakeType;
    boost::asio::ssl::context _sslContext;
    SocketPtr _socket;

    // data to rebuild message
    qi::Message         _msg;
    bool                _connecting;
    std::deque<Message> _sendQueue;
    bool                _sending;
    mutable boost::recursive_mutex _mutex;
    boost::shared_ptr<boost::asio::ip::tcp::resolver> _r;
    bool _isReading;
  };

  using TcpTransportSocketPtr = boost::shared_ptr<TcpTransportSocket>;

}

#endif  // _SRC_TCPTRANSPORTSOCKET_HPP_
