#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_TRANSPORTSERVERASIO_P_HPP_
#define _SRC_TRANSPORTSERVERASIO_P_HPP_

# include <boost/asio.hpp>
# include <boost/asio/ssl.hpp>

# include <qimessaging/api.hpp>
# include <qimessaging/url.hpp>
# include "transportserver_p.hpp"


# include <string>
# include <queue>

struct evconnlistener;

namespace qi
{
  class TransportServerAsioPrivate : public TransportServerPrivate
  {
  public:
    TransportServerAsioPrivate(TransportServer* self,
                                   const qi::Url &url,
                                   EventLoop* ctx,
                                   bool ssl = false);
    virtual ~TransportServerAsioPrivate();

    virtual bool listen();
    virtual void close();
    virtual void destroy();
    boost::asio::ip::tcp::acceptor _acceptor;
  private:
    void onAccept(const boost::system::error_code& erc,
      boost::asio::ssl::stream<boost::asio::ip::tcp::socket>* s,
      boost::shared_ptr<bool> live);
    TransportServerAsioPrivate();
    boost::shared_ptr<bool> _live;
    boost::asio::ssl::context _sslContext;
    bool _ssl;
  };
}

#endif  // _SRC_TRANSPORTSERVERASIO_P_HPP_
