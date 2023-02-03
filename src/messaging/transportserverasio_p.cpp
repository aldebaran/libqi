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
#include <ka/uri/io.hpp>
#include <qi/log.hpp>
#include <qi/eventloop.hpp>
#include <qi/messaging/tcpscheme.hpp>
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
  constexpr const TcpScheme defaultTcpScheme = TcpScheme::Raw;

  static void _onAccept(TransportServerImplPtr p,
                 const boost::system::error_code& erc,
                 boost::asio::ip::tcp::socket peer)
  {
    boost::shared_ptr<TransportServerAsioPrivate> ts = boost::dynamic_pointer_cast<TransportServerAsioPrivate>(p);
    ts->onAccept(erc, std::move(peer));
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

  void TransportServerAsioPrivate::onAccept(const boost::system::error_code& erc, boost::asio::ip::tcp::socket peer)
  {
    qiLogDebug() << this << " onAccept";
    boost::mutex::scoped_lock lock(_acceptCloseMutex);
    if (!_live)
    {
      return;
    }
    if (erc)
    {
      qiLogDebug() << "accept error " << erc.message();
      peer.shutdown(boost::asio::socket_base::shutdown_both);
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
      using N = sock::NetworkAsio;
      using Method = sock::Method<sock::SslContext<N>>;
      auto sslContext = sock::makeSslContextPtr<sock::NetworkAsio>(Method::tlsv12);

      setCipherListTls12AndBelow<N>(*sslContext, N::serverCipherList());

      // Protocols are explicitly forbidden to allow TLS 1.2 only.
      // A white list interface would be preferable, but `Asio` lets us no
      // choice.
      sslContext->set_options(
          boost::asio::ssl::context::default_workarounds
        | boost::asio::ssl::context::no_sslv2
        | boost::asio::ssl::context::no_sslv3
        | boost::asio::ssl::context::no_tlsv1
        | boost::asio::ssl::context::no_tlsv1_1
      );

      sslContext->use_tmp_dh(boost::asio::const_buffer(
        diffieHellmanParameters(),
        std::strlen(diffieHellmanParameters())
      ));

      const auto mode = sock::sslVerifyMode<N>(sock::HandshakeSide<sock::SslSocket<N>>::server, _tcpScheme);
      sslContext->set_verify_mode(mode);

      const auto peerEndpoint = peer.remote_endpoint();
      const auto peerEndpointUrl = sock::url(peerEndpoint, _tcpScheme);

      N::applyConfig(*sslContext, self->_sslConfig, peerEndpoint);

      const auto ioServicePtr = asIoServicePtr(context);
      auto sslSocket = sock::makeSocketWithContextPtr<N>(std::move(peer), std::move(sslContext));
      auto socket = boost::make_shared<qi::TcpMessageSocket<>>(std::move(sslSocket), _tcpScheme, *ioServicePtr);
      qiLogDebug() << "New socket accepted: " << socket.get();

      self->newConnection(std::pair<MessageSocketPtr, Url>{socket, peerEndpointUrl});

      if (socket.unique()) {
        qiLogError() << "bug: socket not stored by the newConnection handler (usecount:" << socket.use_count() << ")";
      }
    }

    _acceptor->async_accept(boost::bind(_onAccept, shared_from_this(), _1, _2));
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
    std::vector<qi::Uri> currentEndpoints;

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

    const auto scheme = to_string(_tcpScheme);
    {
      for (const auto& interface : ifsMap)
      {
        for (const auto& address : interface.second)
        {
          std::ostringstream ss;
          ss << scheme << "://" << address << ":" << _port;
          currentEndpoints.push_back(*qi::uri(ss.str()));
        }
      }
    }

    {
      boost::mutex::scoped_lock l(_endpointsMutex);
      if (currentEndpoints != _endpoints)
      {
        std::ostringstream ss;
        for (const auto& ep : currentEndpoints)
          ss << "ep: " << ep << std::endl;
        qiLogVerbose() << "Updating endpoints..." << this << std::endl << ss.str();
        _endpoints = currentEndpoints;
        _self->endpointsChanged();
      }
      std::sort(_endpoints.begin(), _endpoints.end(), &isPreferredEndpoint);
    }

    *_asyncEndpoints = updateEP();
  }

  qi::Future<void> TransportServerAsioPrivate::listen(const qi::Url& url)
  {
    qiLogCategory("qimessaging.server.listen");

    _listenUrl = url;
    _tcpScheme = tcpScheme(url).value_or(defaultTcpScheme);
    using namespace boost::asio;
#ifndef ANDROID
    // resolve endpoint
    ip::tcp::resolver r(*asIoServicePtr(context));
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
      qiLogError() << ec.message();
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
      _endpoints.push_back(*qi::uri(_listenUrl.str()));
    }
    else
    {
      updateEndpoints();
    }

    {
      boost::mutex::scoped_lock l(_endpointsMutex);
      for (const auto& ep : _endpoints)
      {
        qiLogVerbose() << "TransportServer will listen on: " << ep;
      }
    }

    if (isWithTls(_tcpScheme))
    {
      const auto& certWithPrivKey = self->_sslConfig.certWithPrivKey;
      if (!certWithPrivKey || certWithPrivKey->certificateChain.empty() || !certWithPrivKey->privateKey)
      {
        const char* s = "SSL certificates/key missing, the SSL configuration is incomplete";
        qiLogError() << s;
        return qi::makeFutureError<void>(s);
      }
    }

    _acceptor->async_accept(boost::bind(_onAccept, shared_from_this(), _1, _2));
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
    , _tcpScheme(defaultTcpScheme)
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
