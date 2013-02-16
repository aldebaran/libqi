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

# include <qimessaging/api.hpp>
# include <qimessaging/url.hpp>
# include "transportserver_p.hpp"


# include <string>
# include <queue>

struct evconnlistener;

namespace qi
{
  class TransportServerAsioPrivate : public TransportServerImplPrivate
  {
  public:
    TransportServerAsioPrivate(TransportServer* self,
                                   EventLoop* ctx);
    virtual ~TransportServerAsioPrivate();

    virtual qi::Future<void> listen(const qi::Url& listenUrl);
    virtual void close();
    virtual void destroy();
    boost::asio::ip::tcp::acceptor _acceptor;
  private:
    void onAccept(const boost::system::error_code& erc,
#ifdef WITH_SSL
      boost::asio::ssl::stream<boost::asio::ip::tcp::socket>* s,
#else
      boost::asio::ip::tcp::socket* s,
#endif
      boost::shared_ptr<bool> live);
    TransportServerAsioPrivate();
    boost::shared_ptr<bool> _live;
#ifdef WITH_SSL
    boost::asio::ssl::context _sslContext;
#endif
    bool _ssl;
  };
}

#endif  // _SRC_TRANSPORTSERVERASIO_P_HPP_
