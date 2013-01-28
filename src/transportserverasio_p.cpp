/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <queue>
#include <qi/log.hpp>
#include <cerrno>

#include <boost/asio.hpp>

#include <boost/lexical_cast.hpp>

#include <qimessaging/transportserver.hpp>
#include <qimessaging/transportsocket.hpp>
#include "tcptransportsocket.hpp"

#include <qi/eventloop.hpp>

#include "transportserver_p.hpp"
#include "transportserverasio_p.hpp"

namespace qi
{

  void TransportServerAsioPrivate::onAccept(const boost::system::error_code& erc,
    boost::asio::ip::tcp::socket* s,
    boost::shared_ptr<bool> live)
  {
    qiLogDebug("qimessaging.server.listen") << this << " onAccept";
    if (!(*live))
    {
      delete s;
      return;
    }
    if (erc)
    {
      qiLogDebug("qimessaging.server.listen") << "accept error " << erc.message();
      delete s;
      self->acceptError(erc.value());
      return;
    }
    qi::TransportSocketPtr socket = qi::TcpTransportSocketPtr(new TcpTransportSocket(s, context));
    self->newConnection(socket);
    s = new boost::asio::ip::tcp::socket(_acceptor.get_io_service());
    _acceptor.async_accept(*s,
      boost::bind(&TransportServerAsioPrivate::onAccept, this, _1, s, live));
  }

  void TransportServerAsioPrivate::close() {
    qiLogDebug("qimessaging.server.listen") << this << " close";
    *_live = false;
    _acceptor.close();
  }

  bool TransportServerAsioPrivate::listen()
  {
    using namespace boost::asio;
    // resolve endpoint
    ip::tcp::resolver r(_acceptor.get_io_service());
    ip::tcp::resolver::query q(listenUrl.host(), boost::lexical_cast<std::string>(listenUrl.port()));
    ip::tcp::resolver::iterator it = r.resolve(q);
    if (it == ip::tcp::resolver::iterator())
    {
      qiLogError("qimessaging.server.listen") << "Listen error: no endpoint.";
      return false;
    }
    ip::tcp::endpoint ep = *it;
    qiLogDebug("qimessaging.server.listen") <<"Will listen on " << ep.address().to_string() << ' ' << ep.port();
    _acceptor.open(ep.protocol());
    boost::asio::socket_base::reuse_address option(true);
    _acceptor.set_option(option);
    _acceptor.bind(ep);
    _acceptor.listen();
    unsigned port = _acceptor.local_endpoint().port();// already in host byte orde
    qiLogDebug("qimessaging.server.listen") << "Effective port io_service" << port;
    if (listenUrl.port() == 0)
    {
      listenUrl = Url(listenUrl.protocol() + "://" + listenUrl.host() + ":"
        + boost::lexical_cast<std::string>(port));
    }
    /* Set endpoints */
    if (listenUrl.host() != "0.0.0.0")
    {
      qiLogDebug("qimessaging.server.listen") << "Adding endpoint : " << listenUrl.str();
      _endpoints.push_back(listenUrl.str());
    }

    if (listenUrl.host() == "0.0.0.0") // need available ip addresses
    {
      std::string protocol;
      std::map<std::string, std::vector<std::string> > ifsMap = qi::os::hostIPAddrs();
      if (ifsMap.empty())
      {
        qiLogWarning("qimessaging.server.listen") << "Cannot get host addresses";
        return false;
      }
  #ifdef WIN32 // hostIPAddrs doesn't return loopback on windows
      ifsMap["Loopback"].push_back("127.0.0.1");
  #endif

      protocol = "tcp://";

      for (std::map<std::string, std::vector<std::string> >::iterator interfaceIt = ifsMap.begin();
           interfaceIt != ifsMap.end();
           ++interfaceIt)
      {
        for (std::vector<std::string>::iterator addressIt = (*interfaceIt).second.begin();
             addressIt != (*interfaceIt).second.end();
             ++addressIt)
        {
          std::stringstream ss;
          ss << protocol;
          ss << (*addressIt);
          ss << ":";
          ss << port;
          qiLogVerbose("qimessaging.server.listen") << "Adding endpoint : " << ss.str();
          _endpoints.push_back(ss.str());
         }
      }
    }
    boost::asio::ip::tcp::socket* s = new boost::asio::ip::tcp::socket(_acceptor.get_io_service());
    _acceptor.async_accept(*s,
      boost::bind(&TransportServerAsioPrivate::onAccept, this, _1, s, _live));
    return true;
  }

  void TransportServerAsioPrivate::destroy()
  {
    *_live = false;
    close();
    // We no longuer hold the eventLoop, so we cannot use post.
    // But synchronous deletion of this is safe, since async callback uses _live
    delete this;
  }

  TransportServerAsioPrivate::TransportServerAsioPrivate(TransportServer* self,
                                                                 const qi::Url &url,
                                                                 EventLoop* ctx)
    : TransportServerPrivate(self, url, ctx)
    , _acceptor(*(boost::asio::io_service*)ctx->nativeHandle())
    , _live(new bool(true))
  {
  }

  TransportServerAsioPrivate::~TransportServerAsioPrivate()
  {
    *_live = false;
  }
}
