#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_TRANSPORTSERVERASIO_P_HPP_
#define _SRC_TRANSPORTSERVERASIO_P_HPP_

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/thread/synchronized_value.hpp>
#include <atomic>

# include <qi/api.hpp>
# include <qi/url.hpp>
# include <qi/messaging/tcpscheme.hpp>
# include "sock/networkasio.hpp"
# include "sock/traits.hpp"
# include "sock/socketptr.hpp"
# include "transportserver.hpp"

namespace qi
{
  class TransportServerAsioPrivate:
      public TransportServerImpl,
      public boost::enable_shared_from_this<TransportServerAsioPrivate>
  {
    TransportServerAsioPrivate(TransportServer* self, EventLoop* ctx);

  public:
    static boost::shared_ptr<TransportServerAsioPrivate> make(
        TransportServer* self,
        EventLoop* ctx);

    TransportServerAsioPrivate();
    virtual ~TransportServerAsioPrivate();

    virtual qi::Future<void> listen(const qi::Url& listenUrl);
    virtual void close();
    void updateEndpoints();
    void onAccept(const boost::system::error_code& erc, boost::asio::ip::tcp::socket peer);
    static bool isFatalAcceptError(int errorCode);

    TransportServer* _self;
    boost::asio::ip::tcp::acceptor* _acceptor;
    std::atomic<bool> _live;
    TcpScheme _tcpScheme;
    unsigned short _port;
    boost::synchronized_value<qi::Future<void>> _asyncEndpoints;
    Url _listenUrl;

    // The server must avoid being closed while accepting a connection.
    // Typically, the TransportServer this class has a pointer to closes implementations
    // in its destructor. Without protection, this class can end up using a
    // dangling pointer on the TransportServer.
    boost::mutex _acceptCloseMutex;

    static const int64_t AcceptDownRetryTimerUs;

  private:
    void restartAcceptor();
  };
}

#endif  // _SRC_TRANSPORTSERVERASIO_P_HPP_
