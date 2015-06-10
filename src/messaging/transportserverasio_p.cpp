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

#include "transportserver.hpp"
#include "transportsocket.hpp"
#include "tcptransportsocket.hpp"

#include <qi/eventloop.hpp>

#include "transportserverasio_p.hpp"

qiLogCategory("qimessaging.transportserver");

namespace qi
{
  const int ifsMonitoringTimeout = 5 * 1000 * 1000; // in usec
  const int64_t TransportServerAsioPrivate::AcceptDownRetryTimerUs = 60 * 1000 * 1000; // 60 seconds in usec

  void _onAccept(TransportServerImplPtr p,
                 const boost::system::error_code& erc,
#ifdef WITH_SSL
                 boost::asio::ssl::stream<boost::asio::ip::tcp::socket>* s
#else
                 boost::asio::ip::tcp::socket* s
#endif
                 )
  {
    boost::shared_ptr<TransportServerAsioPrivate> ts = boost::dynamic_pointer_cast<TransportServerAsioPrivate>(p);
    ts->onAccept(erc, s);
  }

  void TransportServerAsioPrivate::restartAcceptor()
  {
    qiLogDebug() << this << " Attempting to restart acceptor";

    if (!_live)
      return;

    if (context)
    {
      _acceptor = new boost::asio::ip::tcp::acceptor(*(boost::asio::io_service*)context->nativeHandle());
      listen(_listenUrl);
    }
    else
      qiLogWarning() << this << " No context available, acceptor will stay down.";
  }

  void TransportServerAsioPrivate::onAccept(const boost::system::error_code& erc,
#ifdef WITH_SSL
    boost::asio::ssl::stream<boost::asio::ip::tcp::socket>* s
#else
    boost::asio::ip::tcp::socket* s
#endif
    )
  {
    qiLogDebug() << this << " onAccept";
    if (!_live)
    {
      delete s;
      return;
    }
    if (erc)
    {
      qiLogDebug() << "accept error " << erc.message();
      delete s;
      self->acceptError(erc.value());
      if (isFatalAcceptError(erc.value()))
      {
        delete _acceptor;
        _acceptor = 0;
        qiLogError() << "fatal accept error: " << erc.value();
        qiLogDebug() << this << " Disabling acceptor for now, retrying in " << AcceptDownRetryTimerUs << "us";
        context->async(boost::bind(&TransportServerAsioPrivate::restartAcceptor, this), AcceptDownRetryTimerUs);
        return;
      }
    }
    else
    {
        qi::TransportSocketPtr socket = qi::TcpTransportSocketPtr(new TcpTransportSocket(context, _ssl, s));
        self->newConnection(socket);

        if (socket.unique()) {
            qiLogError() << "bug: socket not stored by the newConnection handler (usecount:" << socket.use_count() << ")";
        }
    }
#ifdef WITH_SSL
    _s = new boost::asio::ssl::stream<boost::asio::ip::tcp::socket>(_acceptor->get_io_service(), _sslContext);
#else
    _s = new boost::asio::ip::tcp::socket(_acceptor->get_io_service());
#endif
    _acceptor->async_accept(_s->lowest_layer(),
                           boost::bind(_onAccept, shared_from_this(), _1, _s));
  }

  void TransportServerAsioPrivate::close() {
    qiLogDebug() << this << " close";
    try
    {
      _asyncEndpoints.cancel();
    }
    catch (const std::runtime_error& e)
    {
      qiLogDebug() << e.what();
    }

    _live = false;
    if (_acceptor)
      _acceptor->close();
  }

  /*
   * This asynchronous call will keep a shared ptr on the object to prevent
   * its destruction.
   */
  void _updateEndpoints(TransportServerImplPtr p)
  {
    boost::shared_ptr<TransportServerAsioPrivate> ts = boost::dynamic_pointer_cast<TransportServerAsioPrivate>(p);
    ts->updateEndpoints();
  }

  /*
   * This function is used to detect and update endpoints when the transport
   * server is listening on 0.0.0.0.
   */
  void TransportServerAsioPrivate::updateEndpoints()
  {
    if (!_live)
    {
      return;
    }

    // TODO: implement OS networking notifications

    qiLogDebug() << "Checking endpoints...";
    std::vector<qi::Url> currentEndpoints;

    std::map<std::string, std::vector<std::string> > ifsMap = qi::os::hostIPAddrs();
    if (ifsMap.empty())
    {
      const char* s = "Cannot get host addresses";
      qiLogWarning() << s;
    }

    std::string protocol = _ssl ? "tcps://" : "tcp://";

    {
      for (std::map<std::string, std::vector<std::string> >::iterator interfaceIt = ifsMap.begin();
           interfaceIt != ifsMap.end();
           ++interfaceIt)
      {
        for (std::vector<std::string>::iterator addressIt = (*interfaceIt).second.begin();
             addressIt != (*interfaceIt).second.end();
             ++addressIt)
        {
          std::stringstream ss;
          ss << protocol << (*addressIt) << ":" << _port;
          currentEndpoints.push_back(ss.str());
        }
      }
    }

    {
      boost::mutex::scoped_lock l(_endpointsMutex);
      if (_endpoints.size() != currentEndpoints.size() ||
          !std::equal(_endpoints.begin(), _endpoints.end(), currentEndpoints.begin()))
      {
        std::stringstream ss;
        std::vector<qi::Url>::iterator it;
        for (it = currentEndpoints.begin(); it != currentEndpoints.end(); ++it)
          ss << "ep: " << it->str() << std::endl;
        qiLogVerbose() << "Updating endpoints..." << this << std::endl << ss.str();
        _endpoints = currentEndpoints;
        _self->endpointsChanged();
      }

    }

    _asyncEndpoints = context->async(boost::bind(_updateEndpoints, shared_from_this()),
                                     ifsMonitoringTimeout);
  }

  qi::Future<void> TransportServerAsioPrivate::listen(const qi::Url& url)
  {
    _listenUrl = url;
    _ssl = _listenUrl.protocol() == "tcps";
    using namespace boost::asio;
#ifndef ANDROID
    // resolve endpoint
    ip::tcp::resolver r(_acceptor->get_io_service());
    ip::tcp::resolver::query q(_listenUrl.host(), boost::lexical_cast<std::string>(_listenUrl.port()),
                               boost::asio::ip::tcp::resolver::query::all_matching);
    ip::tcp::resolver::iterator it = r.resolve(q);

    static bool disableIPV6 = qi::os::getenv("QIMESSAGING_ENABLE_IPV6").empty();
    if (disableIPV6)
    {
      while (it != boost::asio::ip::tcp::resolver::iterator() &&
             it->endpoint().address().is_v6())
        ++it;
    }
    if (it == ip::tcp::resolver::iterator())
    {
      const char* s = "Listen error: no valid endpoint.";
      qiLogError() << s;
      return qi::makeFutureError<void>(s);
    }


    ip::tcp::endpoint ep = *it;
#else
    ip::tcp::endpoint ep(boost::asio::ip::address::from_string(url.host()), url.port());
#endif // #ifndef ANDROID

    qiLogDebug() << "Will listen on " << ep.address().to_string() << ' ' << ep.port();
    _acceptor->open(ep.protocol());
#ifdef _WIN32
    boost::asio::socket_base::reuse_address option(false);
#else
    boost::asio::socket_base::reuse_address option(true);
    fcntl(_acceptor->native(), F_SETFD, FD_CLOEXEC);
#endif
    _acceptor->set_option(option);
    _acceptor->bind(ep);
    boost::system::error_code ec;
    _acceptor->listen(socket_base::max_connections, ec);
    if (ec)
    {
      qiLogError("qimessaging.server.listen") << ec.message();
      return qi::makeFutureError<void>(ec.message());
    }
    _port = _acceptor->local_endpoint().port();// already in host byte orde
    qiLogDebug() << "Effective port io_service" << _port;
    if (_listenUrl.port() == 0)
    {
      _listenUrl = Url(_listenUrl.protocol() + "://" + _listenUrl.host() + ":"
        + boost::lexical_cast<std::string>(_port));
    }

    /* Set endpoints */
    if (_listenUrl.host() != "0.0.0.0")
    {
      boost::mutex::scoped_lock l(_endpointsMutex);
      _endpoints.push_back(_listenUrl.str());
    }
    else
    {
      updateEndpoints();
    }

    {
      boost::mutex::scoped_lock l(_endpointsMutex);
      for (std::vector<qi::Url>::const_iterator it = _endpoints.begin();
           it != _endpoints.end();
           it++)
      {
        qiLogInfo() << "TransportServer will listen on: " << it->str();
      }
    }

#ifdef WITH_SSL
    if (_ssl)
    {
      if (self->_identityCertificate.empty() || self->_identityKey.empty())
      {
        const char* s = "SSL certificates missing, please call Session::setIdentity first";
        qiLogError("qimessaging.server.listen") << s;
        return qi::makeFutureError<void>(s);
      }

      _sslContext.set_options(
        boost::asio::ssl::context::default_workarounds
        | boost::asio::ssl::context::no_sslv2);
      _sslContext.use_certificate_chain_file(self->_identityCertificate.c_str());
      _sslContext.use_private_key_file(self->_identityKey.c_str(), boost::asio::ssl::context::pem);
    }

    _s = new boost::asio::ssl::stream<boost::asio::ip::tcp::socket>(_acceptor->get_io_service(), _sslContext);
#else
    _s = new boost::asio::ip::tcp::socket(_acceptor->get_io_service());
#endif
    _acceptor->async_accept(_s->lowest_layer(),
      boost::bind(_onAccept, shared_from_this(), _1, _s));
    _connectionPromise.setValue(0);
    return _connectionPromise.future();
  }

  bool TransportServerAsioPrivate::isFatalAcceptError(int errorCode)
  {
    using namespace boost::system::errc;
    static const errc_t fatalErrors[] =
    {
      bad_file_descriptor,
      bad_address,
      invalid_argument,
      not_a_socket,
      operation_not_supported,
      protocol_error,
      operation_not_permitted,
      connection_reset,
      network_down,
    };
    for (size_t i = 0; i < sizeof(fatalErrors) / sizeof(fatalErrors[0]); ++i)
      if (errorCode == fatalErrors[i])
        return true;
    return false;
  }

  TransportServerAsioPrivate::TransportServerAsioPrivate(TransportServer* self,
                                                                 EventLoop* ctx)
    : TransportServerImpl(self, ctx)
    , _self(self)
    , _acceptor(new boost::asio::ip::tcp::acceptor(*(boost::asio::io_service*)ctx->nativeHandle()))
    , _live(true)
#ifdef WITH_SSL
    , _sslContext(*(boost::asio::io_service*)ctx->nativeHandle(), boost::asio::ssl::context::sslv23)
#endif
    , _s(NULL)
    , _ssl(false)
    , _port(0)
  {
  }

  TransportServerAsioPrivate::~TransportServerAsioPrivate()
  {
    delete _acceptor;
    _acceptor = 0;
  }
}
