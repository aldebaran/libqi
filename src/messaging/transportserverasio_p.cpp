/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include <string>
#include <cstring>
#include <cstdlib>
#include <queue>
#include <cerrno>
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <ka/memory.hpp>
#include <qi/log.hpp>
#include <qi/eventloop.hpp>
#include "transportserver.hpp"
#include "messagesocket.hpp"
#include "tcpmessagesocket.hpp"
#include "transportserverasio_p.hpp"
#include "sock/traits.hpp"
#include "sock/sslcontextptr.hpp"

qiLogCategory("qimessaging.transportserver");

namespace qi
{
  const int ifsMonitoringTimeout = 5 * 1000 * 1000; // in usec
  const int64_t TransportServerAsioPrivate::AcceptDownRetryTimerUs = 60 * 1000 * 1000; // 60 seconds in usec

  void _onAccept(TransportServerImplPtr p,
                 const boost::system::error_code& erc,
                 sock::SocketWithContextPtr<sock::NetworkAsio> s
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
    sock::SocketWithContextPtr<sock::NetworkAsio> s
    )
  {
    qiLogDebug() << this << " onAccept";
    boost::mutex::scoped_lock lock(_acceptCloseMutex);
    if (!_live)
    {
      s.reset();
      return;
    }
    if (erc)
    {
      qiLogDebug() << "accept error " << erc.message();
      s.reset();
      self->acceptError(erc.value());
      if (isFatalAcceptError(erc.value()))
      {
        delete _acceptor;
        _acceptor = 0;
        qiLogError() << "fatal accept error: " << erc.value();
        qiLogDebug() << this << " Disabling acceptor for now, retrying in " << AcceptDownRetryTimerUs << "us";
        context->asyncDelay(boost::bind(&TransportServerAsioPrivate::restartAcceptor, this),
            qi::MicroSeconds(AcceptDownRetryTimerUs));
        return;
      }
    }
    else
    {
        auto socket = boost::make_shared<qi::TcpMessageSocket<>>(*asIoServicePtr(context), _ssl, s);
        qiLogDebug() << "New socket accepted: " << socket.get();

        self->newConnection(std::pair<MessageSocketPtr, Url>{
          socket, sock::remoteEndpoint(*s, _ssl)});

        if (socket.unique()) {
            qiLogError() << "bug: socket not stored by the newConnection handler (usecount:" << socket.use_count() << ")";
        }
    }
    _s = sock::makeSocketWithContextPtr<sock::NetworkAsio>(_acceptor->get_io_service(), _sslContext);
    _acceptor->async_accept(_s->lowest_layer(),
                           boost::bind(_onAccept, shared_from_this(), _1, _s));
  }

  void TransportServerAsioPrivate::close() {
    qiLogDebug() << this << " close";
    boost::mutex::scoped_lock l(_acceptCloseMutex);
    try
    {
      _asyncEndpoints->cancel();
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
    boost::shared_ptr<TransportServerAsioPrivate> ts = boost::static_pointer_cast<TransportServerAsioPrivate>(p);
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

    auto updateEP = [&]
    {
      return context->asyncDelay(boost::bind(_updateEndpoints, shared_from_this()),
                                 qi::MicroSeconds(ifsMonitoringTimeout));
    };

    std::map<std::string, std::vector<std::string> > ifsMap = qi::os::hostIPAddrs();
    if (ifsMap.empty())
    {
      const char* s = "Cannot get host addresses";
      qiLogWarning() << s;
      *_asyncEndpoints = updateEP();
      return;
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

    *_asyncEndpoints = updateEP();
  }

  // Diffie-Hellman parameters of size 3072 bits at `PEM` format.
  //
  // Generation example: `openssl dhparam -outform PEM -out dhparams.pem 3072`
  static char const* diffieHellmanParameters()
  {
    // Hardcoded for security reasons.
    return
      "-----BEGIN DH PARAMETERS-----\n"
      "MIIBCAKCAQEAxtYYmi/o+PGc1/j5SLb/kPRjSJj2ZlnFHI6OxYjU1sr36KuOz6x+\n"
      "MuRYnUcHChATpZ88lbhyw7w/L+R+E9uuOkkU78zNXv5xsElsKfl2BQpdO8N+h3y4\n"
      "SlN3P8F8TmAgmRG/LZaABycWONSeRXGPZ76dE79z+0UE+Ae7jHqRCfjsudpb/DfB\n"
      "JnnMYOsONQnpYywEjIJ6H5voGoYR5QLZPqBRHZuZTb8cBg5psPzIIHB3h77f6Xe8\n"
      "irSYKhLmM3WqwDPnIZq+NdBrYZOziOrQqTdtcuPTtxYhhodIHhDK9213/onRSqft\n"
      "mOEHq1J6vhjsv1mCAcZDvJYRQEFN3/gSywIBAg==\n"
      "-----END DH PARAMETERS-----";
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

    qiLogDebug() << "Will listen on " << ep;
    _acceptor->open(ep.protocol());
#ifdef _WIN32
    boost::asio::socket_base::reuse_address option(false);
#else
    boost::asio::socket_base::reuse_address option(true);
    fcntl(_acceptor->native_handle(), F_SETFD, FD_CLOEXEC);
#endif
    _acceptor->set_option(option);
    try
    {
      _acceptor->bind(ep);
    }
    catch (const boost::system::system_error& e)
    {
      std::stringstream ss;
      ss << "failed to listen on " << ep << ": " << e.what();
      throw std::runtime_error(ss.str());
    }

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
        qiLogVerbose() << "TransportServer will listen on: " << it->str();
      }
    }

    if (_ssl)
    {
      if (self->_identityCertificate.empty() || self->_identityKey.empty())
      {
        const char* s = "SSL certificates missing, please call Session::setIdentity first";
        qiLogError("qimessaging.server.listen") << s;
        return qi::makeFutureError<void>(s);
      }

      using N = sock::NetworkAsio;
      setCipherListTls12AndBelow<N>(*_sslContext, N::serverCipherList());

      // Protocols are explicitly forbidden to allow TLS 1.2 only.
      // A white list interface would be preferable, but `Asio` lets us no
      // choice.
      _sslContext->set_options(
          boost::asio::ssl::context::default_workarounds
        | boost::asio::ssl::context::no_sslv2
        | boost::asio::ssl::context::no_sslv3
        | boost::asio::ssl::context::no_tlsv1
        | boost::asio::ssl::context::no_tlsv1_1
      );
      _sslContext->use_certificate_chain_file(self->_identityCertificate.c_str());
      _sslContext->use_private_key_file(self->_identityKey.c_str(), boost::asio::ssl::context::pem);

      _sslContext->use_tmp_dh(boost::asio::const_buffer(
        diffieHellmanParameters(),
        std::strlen(diffieHellmanParameters())
      ));
    }

    _s = sock::makeSocketWithContextPtr<sock::NetworkAsio>(_acceptor->get_io_service(), _sslContext);
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
    , _acceptor(new boost::asio::ip::tcp::acceptor(*asIoServicePtr(ctx)))
    , _live(true)
    , _sslContext(sock::makeSslContextPtr<sock::NetworkAsio>(
                    sock::SslContext<sock::NetworkAsio>::tlsv12))
    , _s()
    , _ssl(false)
    , _port(0)
  {
  }

  boost::shared_ptr<TransportServerAsioPrivate> TransportServerAsioPrivate::make(
      TransportServer* self,
      EventLoop* ctx)
  {
    return boost::shared_ptr<TransportServerAsioPrivate>{new TransportServerAsioPrivate(self, ctx)};
  }

  TransportServerAsioPrivate::~TransportServerAsioPrivate()
  {
    delete _acceptor;
    _acceptor = 0;
  }
}
