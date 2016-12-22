#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_TRANSPORTSERVERASIO_P_HPP_
#define _SRC_TRANSPORTSERVERASIO_P_HPP_

# include <boost/asio.hpp>
# include <boost/asio/ssl.hpp>
#include <boost/thread/synchronized_value.hpp>
#include <atomic>

# include <qi/api.hpp>
# include <qi/url.hpp>
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

    virtual ~TransportServerAsioPrivate();

    virtual qi::Future<void> listen(const qi::Url& listenUrl);
    virtual void close();
    void updateEndpoints();
    static bool isFatalAcceptError(int errorCode);
    TransportServer* _self;
    boost::asio::ip::tcp::acceptor* _acceptor;
    void onAccept(const boost::system::error_code& erc,
      boost::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> s);
    TransportServerAsioPrivate();
    std::atomic<bool> _live;
    boost::asio::ssl::context _sslContext;
    boost::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> _s;
    bool _ssl;
    unsigned short _port;
    boost::synchronized_value<qi::Future<void>> _asyncEndpoints;
    Url _listenUrl;

    static const int64_t AcceptDownRetryTimerUs;

  private:
    void restartAcceptor();
  };
}

#endif  // _SRC_TRANSPORTSERVERASIO_P_HPP_
