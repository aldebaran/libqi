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
    boost::asio::ssl::stream<boost::asio::ip::tcp::socket>* s,
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
    qi::TransportSocketPtr socket = qi::TcpTransportSocketPtr(new TcpTransportSocket(s, context, _ssl));
    self->newConnection(socket);
    s = new boost::asio::ssl::stream<boost::asio::ip::tcp::socket>(_acceptor.get_io_service(), _sslContext);
    _acceptor.async_accept(s->lowest_layer(),
      boost::bind(&TransportServerAsioPrivate::onAccept, this, _1, s, live));
  }

  void TransportServerAsioPrivate::close() {
    qiLogDebug("qimessaging.server.listen") << this << " close";
    *_live = false;
    _acceptor.close();
  }

  qi::Future<void> TransportServerAsioPrivate::listen()
  {
    using namespace boost::asio;
    // resolve endpoint
    ip::tcp::resolver r(_acceptor.get_io_service());
    ip::tcp::resolver::query q(listenUrl.host(), boost::lexical_cast<std::string>(listenUrl.port()));
    ip::tcp::resolver::iterator it = r.resolve(q);
    if (it == ip::tcp::resolver::iterator())
    {
      const char* s = "Listen error: no endpoint.";
      qiLogError("qimessaging.server.listen") << s;
      return qi::makeFutureError<void>(s);
    }
    ip::tcp::endpoint ep = *it;
    qiLogDebug("qimessaging.server.listen") <<"Will listen on " << ep.address().to_string() << ' ' << ep.port();
    _acceptor.open(ep.protocol());
#ifdef _WIN32
    boost::asio::socket_base::reuse_address option(false);
#else
    boost::asio::socket_base::reuse_address option(true);
#endif
    _acceptor.set_option(option);
    _acceptor.bind(ep);
    boost::system::error_code ec;
    _acceptor.listen(socket_base::max_connections, ec);
    if (ec)
    {
      qiLogError("qimessaging.server.listen") << ec.message();
      return qi::makeFutureError<void>(ec.message());
    }
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
        const char* s = "Cannot get host addresses";
        qiLogWarning("qimessaging.server.listen") << s;
        return qi::makeFutureError<void>(s);
      }
  #ifdef WIN32 // hostIPAddrs doesn't return loopback on windows
      ifsMap["Loopback"].push_back("127.0.0.1");
  #endif

      protocol = _ssl ? "tcps://" : "tcp://";

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

    if (_ssl)
    {
      if (self->_p->_identityCertificate.empty() || self->_p->_identityKey.empty())
      {
        const char* s = "SSL certificates missing, please call Session::setIdentity first";
        qiLogError("qimessaging.server.listen") << s;
        return qi::makeFutureError<void>(s);
      }

      _sslContext.set_options(
        boost::asio::ssl::context::default_workarounds
        | boost::asio::ssl::context::no_sslv2);
      _sslContext.use_certificate_chain_file(self->_p->_identityCertificate.c_str());
      _sslContext.use_private_key_file(self->_p->_identityKey.c_str(), boost::asio::ssl::context::pem);
    }

    boost::asio::ssl::stream<boost::asio::ip::tcp::socket>* s = new boost::asio::ssl::stream<boost::asio::ip::tcp::socket>(_acceptor.get_io_service(), _sslContext);
    _acceptor.async_accept(s->lowest_layer(),
      boost::bind(&TransportServerAsioPrivate::onAccept, this, _1, s, _live));
    _connectionPromise.setValue(0);
    return _connectionPromise.future();
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
                                                                 EventLoop* ctx,
                                                                 bool ssl)
    : TransportServerImplPrivate(self, url, ctx)
    , _acceptor(*(boost::asio::io_service*)ctx->nativeHandle())
    , _live(new bool(true))
    , _sslContext(*(boost::asio::io_service*)ctx->nativeHandle(), boost::asio::ssl::context::sslv23)
    , _ssl(ssl)
  {
  }

  TransportServerAsioPrivate::~TransportServerAsioPrivate()
  {
    *_live = false;
  }
}
