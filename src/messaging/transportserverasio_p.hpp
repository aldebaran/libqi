#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_TRANSPORTSERVERASIO_P_HPP_
#define _SRC_TRANSPORTSERVERASIO_P_HPP_

# include <boost/asio.hpp>
# ifdef WITH_SSL
# include <boost/asio/ssl.hpp>
# endif

# include <qi/api.hpp>
# include <qi/url.hpp>
# include "transportserver.hpp"

namespace qi
{
  class TransportServerAsioPrivate : public TransportServerImpl
  {
  public:
    TransportServerAsioPrivate(TransportServer* self,
                                   EventLoop* ctx);
    virtual ~TransportServerAsioPrivate();

    virtual qi::Future<void> listen(const qi::Url& listenUrl);
    virtual void close();
    void updateEndpoints();
    static bool isFatalAcceptError(int errorCode);
    TransportServer* _self;
    boost::asio::ip::tcp::acceptor* _acceptor;
    void onAccept(const boost::system::error_code& erc,
#ifdef WITH_SSL
      boost::asio::ssl::stream<boost::asio::ip::tcp::socket>* s
#else
      boost::asio::ip::tcp::socket* s
#endif
      );
    TransportServerAsioPrivate();
    bool _live;
#ifdef WITH_SSL
    boost::asio::ssl::context _sslContext;
    boost::asio::ssl::stream<boost::asio::ip::tcp::socket>* _s;
#else
    boost::asio::ip::tcp::socket* _s;
#endif
    bool _ssl;
    unsigned short _port;
    qi::Future<void> _asyncEndpoints;
    Url _listenUrl;

    static const int64_t AcceptDownRetryTimerUs;

  private:
    void restartAcceptor();
  };
}

#endif  // _SRC_TRANSPORTSERVERASIO_P_HPP_
