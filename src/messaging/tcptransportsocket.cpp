/*
**  Copyright (C) 2012, 2013 Aldebaran Robotics
**  See COPYING for the license
*/

// Disable "'this': used in base member initializer list"
#ifdef _MSC_VER
# pragma warning( push )
# pragma warning(disable: 4355)
#endif

#ifdef _WIN32
#include <Winsock2.h> // needed by mstcpip.h
#include <Mstcpip.h> // for tcp_keepalive struct
#endif

#ifdef ANDROID
#include <linux/in.h> // for  IPPROTO_TCP
#endif

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/make_shared.hpp>

#include "tcptransportsocket.hpp"

#include <qi/log.hpp>
#include <qi/os.hpp>

qiLogCategory("qimessaging.transportsocket");

/**
 * ###
 * connect/disconnect promise could be called multiple times.
 * (error could be handled twice for example)
 * so catch the error and continue, only the first set is taken into account
 */

static void pSetError(qi::Promise<void> prom, const std::string &error) {
  try { //could have already been set.
    prom.setError(error);
  } catch (const qi::FutureException &fe) {
    if (fe.state() != qi::FutureException::ExceptionState_PromiseAlreadySet)
      throw;
    qiLogVerbose() << "Error already set on promise.";
  }
}

static void pSetValue(qi::Promise<void> prom) {
  try { //could have already been set.
    prom.setValue(0);
  } catch (const qi::FutureException &fe) {
    if (fe.state() != qi::FutureException::ExceptionState_PromiseAlreadySet)
      throw;
    qiLogVerbose() << "Value already set on promise.";
  }
}

namespace qi
{
  TcpTransportSocket::TcpTransportSocket(EventLoop* eventLoop, bool ssl, boost::shared_ptr<Socket> s)
    : TransportSocket()
    , _ssl(ssl)
    , _nextHandshakeType()
    , _sslContext(boost::asio::ssl::context::sslv23)
    , _socket(s)
    , _connecting(false)
    , _sending(false)
    , _isReading(false)
  {
    _eventLoop = eventLoop;
    _err = 0;
    _status = qi::TransportSocket::Status::Disconnected;

    if (_socket)
    { // TODO: check that the socket actually is connected
      _status = qi::TransportSocket::Status::Connected;
      if (_ssl)
        _nextHandshakeType = boost::asio::ssl::stream_base::server;
      // Transmit each Message without delay
      setSocketOptions();
    }
  }

  TcpTransportSocket::~TcpTransportSocket()
  {
    qiLogDebug() << this;
    error("Automatic disconnection of destroyed TCP transport socket");
    qiLogVerbose() << "deleted " << this;
  }

  void TcpTransportSocket::ensureReading()
  {
    if (!isConnected())
      throw std::runtime_error("cannot read on a disconnected socket");
    // no connection attempt here (it is already connected), we are not interested
    // in the result of the connection attempt to be set in the promise
    ensureReading(qi::Promise<void>{});
  }

  void TcpTransportSocket::ensureReading(qi::Promise<void> connectionAttemptPromise)
  {
    boost::recursive_mutex::scoped_lock l(_mutex);
    if (!_socket)
      throw std::runtime_error("socket is not connected");
    if (_isReading)
      return;
    _isReading = true;
    _continueReading(connectionAttemptPromise);
  }

  void TcpTransportSocket::_continueReading(qi::Promise<void> connectionAttemptPromise)
  {
    boost::recursive_mutex::scoped_lock l(_mutex);
    _msg = {};

    if (!_socket)
      return;

    if (_ssl)
    {
      if (_nextHandshakeType)
      {
        _socket->async_handshake(_nextHandshakeType.value(),
          boost::bind(&TcpTransportSocket::handshake, shared_from_this(), _1, _socket, connectionAttemptPromise));
        return;
      }

      boost::asio::async_read(*_socket,
        boost::asio::buffer(_msg._p->getHeader(), sizeof(MessagePrivate::MessageHeader)),
        boost::bind(&TcpTransportSocket::onReadHeader, shared_from_this(), _1, _2, _socket));
    }
    else
    {
      boost::asio::async_read(_socket->next_layer(),
        boost::asio::buffer(_msg._p->getHeader(), sizeof(MessagePrivate::MessageHeader)),
        boost::bind(&TcpTransportSocket::onReadHeader, shared_from_this(), _1, _2, _socket));
    }
  }

  qi::Url TcpTransportSocket::remoteEndpoint() const
  {
    boost::recursive_mutex::scoped_lock lock(_mutex);
    if (!_socket)
      return qi::Url();
    return qi::Url(
      _socket->lowest_layer().remote_endpoint().address().to_string(),
      _ssl ? "tcps" : "tcp",
      _socket->lowest_layer().remote_endpoint().port());
  }

  void TcpTransportSocket::onReadHeader(const boost::system::error_code& erc,
    std::size_t len, SocketPtr)
  {
    boost::recursive_mutex::scoped_lock l(_mutex);
    if (erc)
    {
      error("System error: " + erc.message());
      return;
    }

    if (!_socket)
      return;

    // when using SSL, sometimes we are called spuriously
    if (len == 0)
    {
      _continueReading(qi::Promise<void>{});
      return;
    }
    QI_ASSERT(len == sizeof(_msg._p->header));
    // check magic
    if (_msg._p->header.magic != MessagePrivate::magic)
    {
      qiLogWarning() << "Incorrect magic from "
        << _socket->lowest_layer().remote_endpoint().address().to_string()
        << ", disconnecting"
           " (expected " << MessagePrivate::magic
        << ", got " << _msg._p->header.magic << ").";
      error("Protocol error");
      return;
    }

    size_t payload = _msg._p->header.size;
    if (payload)
    {
      static size_t maxPayload = 0;
      static bool init = false;
      // Not thread-safe, limited consequences
      // worst case: first received messages will not honor limit)
      if (!init)
      {
        init = true;
        std::string l = os::getenv("QI_MAX_MESSAGE_PAYLOAD");
        if (!l.empty())
          maxPayload = strtol(l.c_str(), 0, 0);
        else
          maxPayload = 50000000; // reasonable default
      }
      if (maxPayload && payload > maxPayload)
      {
        qiLogWarning() << "Receiving message of size " << payload
          << " above maximum configured payload " << maxPayload << ", closing link."
             " (configure with environment variable QI_MAX_MESSAGE_PAYLOAD)";
        error("Message too big");
        return;
      }

      void* ptr = _msg._p->buffer.reserve(payload);

      if (_ssl)
      {
        boost::asio::async_read(*_socket,
          boost::asio::buffer(ptr, payload),
          boost::bind(&TcpTransportSocket::onReadData, shared_from_this(), _1, _2, _socket));
      }
      else
      {
        boost::asio::async_read(_socket->next_layer(),
          boost::asio::buffer(ptr, payload),
          boost::bind(&TcpTransportSocket::onReadData, shared_from_this(), _1, _2, _socket));
      }
    }
    else
      onReadData(boost::system::error_code(), 0, _socket);
  }

  void TcpTransportSocket::onReadData(const boost::system::error_code& erc,
    std::size_t, SocketPtr)
  {
    if (erc)
    {
      error("System error: " + erc.message());
      return;
    }

    // Timing utilities for dispatch diagnosis
    qi::int64_t start = 0;
    static auto warnThresholdEnv = os::getenv("QIMESSAGING_SOCKET_DISPATCH_TIME_WARN_THRESHOLD");
    static int usWarnThreshold = warnThresholdEnv.empty() ? 0 : strtol(warnThresholdEnv.c_str(), 0, 0);
    if (usWarnThreshold)
      start = os::ustime(); // call might be not that cheap

    Message toDispatch;
    {
      bool mustDispatchMessages = false;
      boost::recursive_mutex::scoped_lock l(_mutex);
      if (!_socket)
        return;

      qiLogDebug() << this << " Recv (" << _msg.type() << "):" << _msg.address();

      if ((!hasReceivedRemoteCapabilities() &&
            _msg.service() == Message::Service_Server &&
            _msg.function() == Message::ServerFunction_Authenticate)
          || _msg.type() == Message::Type_Capability)
      {
        // This one is for us
        if (_msg.type() != Message::Type_Error)
        {
          AnyReference cmRef;
          try
          {
            cmRef = _msg.value(typeOf<CapabilityMap>()->signature(), shared_from_this());
            CapabilityMap cm = cmRef.to<CapabilityMap>();
            cmRef.destroy();
            boost::mutex::scoped_lock lock(_contextMutex);
            _remoteCapabilityMap.insert(cm.begin(), cm.end());
          }
          catch (const std::runtime_error& e)
          {
            cmRef.destroy();
            qiLogError() << "Ill-formed capabilities message: " << e.what();
            return error("Ill-formed capabilities message.");
          }
        }

        if (_msg.type() != Message::Type_Capability)
          mustDispatchMessages = true;
      }
      else
      {
        mustDispatchMessages = true;
      }
      std::swap(toDispatch, _msg);
    } // end critical section

    messageReady(toDispatch);
    socketEvent(SocketEventData(toDispatch));
    _dispatcher.dispatch(toDispatch);

    if (usWarnThreshold)
    {
      qi::int64_t duration = os::ustime() - start;
      if (duration > usWarnThreshold)
        qiLogWarning() << "Dispatch to user took " << duration << "us";
    }
    _continueReading(qi::Promise<void>{});
  }

  void TcpTransportSocket::error(const std::string& erc)
  {
    qiLogVerbose() << "Socket error: " << erc;
    boost::recursive_mutex::scoped_lock lock(_mutex);

    _isReading = false; // does the inverse of ensureReading()

    if (!_socket)
    {
      // Return straight away if error has already been called.
      // Otherwise, `disconnected` callbacks could be called
      // multiple times.
      return;
    }

    const bool wasConnected = _status == qi::TransportSocket::Status::Connected;
    _status = qi::TransportSocket::Status::Disconnected;

    _nextHandshakeType.reset();

    if (_connecting)
      _connecting = false;

    {
      boost::system::error_code er;

      // Unconditionally try to shutdown if socket is present, it might be in connecting state.
      _socket->lowest_layer().shutdown(boost::asio::ip::tcp::socket::shutdown_both, er);
      _socket->lowest_layer().close(er);
    }
    _socket.reset();


    // synchronous signals, do not keep the mutex while we trigger
    if (wasConnected)
      disconnected(erc);

    socketEvent(SocketEventData(erc));
  }

  qi::FutureSync<void> TcpTransportSocket::connect(const qi::Url &url)
  {
    boost::recursive_mutex::scoped_lock l(_mutex);

    if (_status == qi::TransportSocket::Status::Connected || _connecting)
    {
      const char* s = "connection already in progress";
      qiLogError() << s;
      return makeFutureError<void>(s);
    }
    if (_ssl)
    {
      _sslContext.set_verify_mode(boost::asio::ssl::verify_none);
    }
    auto eventLoopAsAsioService = static_cast<boost::asio::io_service*>(_eventLoop->nativeHandle());
    _socket = boost::make_shared<Socket>(*eventLoopAsAsioService, _sslContext);
    _url = url;
    _status = qi::TransportSocket::Status::Connecting;
    _connecting = true;
    _err = 0;
    if (_url.port() == 0 || !url.isValid()) {
      qiLogError() << "Error try to connect to a bad address: " << _url.str();

      _status = qi::TransportSocket::Status::Disconnected;
      _connecting = false;
      return qi::makeFutureError<void>(std::string("Bad address ") + _url.str());
    }
    qiLogVerbose() << "Trying to connect to " << _url.host() << ":" << _url.port();
    using namespace boost::asio;
    // Resolve url
    _r = boost::make_shared<boost::asio::ip::tcp::resolver>(_socket->get_io_service());
    ip::tcp::resolver::query q(_url.host(), boost::lexical_cast<std::string>(_url.port())
                           #ifndef ANDROID
                               , boost::asio::ip::tcp::resolver::query::all_matching
                           #endif
                               );

    qi::Promise<void> connectPromise;
    _r->async_resolve(q,
                      boost::bind(&TcpTransportSocket::onResolved,
                                  shared_from_this(),
                                  boost::asio::placeholders::error,
                                  boost::asio::placeholders::iterator,
                                  _socket,
                                  connectPromise));
    return connectPromise.future();
  }

  void TcpTransportSocket::onResolved(const boost::system::error_code& erc,
                                      boost::asio::ip::tcp::resolver::iterator it,
                                      SocketPtr,
                                      qi::Promise<void> connectPromise)
  {
    boost::recursive_mutex::scoped_lock l(_mutex);
    if (!_socket)
    {
      // Disconnection was requested, so error() was already called.
      pSetError(connectPromise, "Disconnection requested");
      return;
    }
    else if (erc)
    {
      std::string message = "System error: " + erc.message();
      qiLogWarning() << "resolve: " << message;
      error(message);
      _status = qi::TransportSocket::Status::Disconnected;
      pSetError(connectPromise, message);
      return;
    }

    static bool disableIPV6 = qi::os::getenv("QIMESSAGING_ENABLE_IPV6").empty();
    if (disableIPV6)
    {
      while (it != boost::asio::ip::tcp::resolver::iterator() &&
             it->endpoint().address().is_v6())
        ++it;
    }
    if (it == boost::asio::ip::tcp::resolver::iterator())
    {
      std::stringstream s;
      s << "Only IPv6 were resolved on " << url().str();
      qiLogError() << s.str();
      error(s.str());
      pSetError(connectPromise, s.str());
      return;
    }

    // asynchronous connect
    _socket->lowest_layer().async_connect(*it,
                                          boost::bind(&TcpTransportSocket::onConnected,
                                                      shared_from_this(),
                                                      boost::asio::placeholders::error,
                                                      _socket,
                                                      connectPromise));
    _r.reset();
  }

  void TcpTransportSocket::handshake(
      const boost::system::error_code& erc,
      SocketPtr,
      qi::Promise<void> connectPromise)
  {
    boost::recursive_mutex::scoped_lock l(_mutex);
    _nextHandshakeType.reset();

    if (erc)
    {
      qiLogWarning() << "Error connecting " << _url.str() << ": " << erc.message();
      _status = qi::TransportSocket::Status::Disconnected;
      error("System error: " + erc.message());
      pSetError(connectPromise, "System error: " + erc.message());
    }
    else
    {
      // The connection attempt succeeded, so the caller is notified so.
      // But the status should be updated only if no disconnection occurred
      // in the meantime.
      pSetValue(connectPromise);
      if (!_socket)
        return;

      _status = qi::TransportSocket::Status::Connected;
      connected();

      // Transmit each Message without delay
      const boost::asio::ip::tcp::no_delay option( true );
      try
      {
        _socket->lowest_layer().set_option(option);
      } catch (std::exception& e)
      {
        qiLogWarning() << "can't set no_delay option: " << e.what();
      }

      _continueReading(qi::Promise<void>{});
    }
  }

  void TcpTransportSocket::onConnected(
      const boost::system::error_code& erc,
      SocketPtr,
      qi::Promise<void> connectPromise)
  {
    boost::recursive_mutex::scoped_lock l(_mutex);
    _connecting = false;
    if (erc)
    {
      qiLogWarning() << "Error connecting " << _url.str() << ": " << erc.message();
      error("System error: " + erc.message());
      _status = qi::TransportSocket::Status::Disconnected;
      pSetError(connectPromise, "System error: " + erc.message());
    }
    else
    {
      if (!_ssl)
        pSetValue(connectPromise);
      if (_socket)
      {
        if (!_ssl)
        {
          _nextHandshakeType.reset();
          _status = qi::TransportSocket::Status::Connected;
          connected();
        }
        else
        {
          _nextHandshakeType = boost::asio::ssl::stream_base::client;
        }

        setSocketOptions();
        ensureReading(connectPromise);
      }
      else
      {
        if (_ssl)
          pSetError(connectPromise, "Disconnected while connecting");
      }
    }
  }

  void TcpTransportSocket::setSocketOptions()
  {
    assert(_socket);
    // Transmit each Message without delay
    const boost::asio::ip::tcp::no_delay option( true );
    try {
      _socket->lowest_layer().set_option(option);
    } catch (std::exception& e)
    {
      qiLogWarning() << "can't set no_delay option: " << e.what();
    }

    // Enable TCP keepalive for faster timeout detection.
    static const char* envTimeout = getenv("QI_TCP_PING_TIMEOUT");
    int timeout = 30;
    if (envTimeout)
      timeout = strtol(envTimeout, 0, 0);
    if (!timeout)
      return; // feature disabled
    // we can't honor timeout < 10s proprely
    timeout = std::max(timeout, 10);
    boost::asio::ip::tcp::socket::native_handle_type handle
      = _socket->lowest_layer().native_handle();
#ifdef _WIN32
    /* http://msdn.microsoft.com/en-us/library/windows/desktop/dd877220(v=vs.85).aspx
    On Windows Vista and later, the number of keep-alive probes (data retransmissions) is set to 10 and cannot be changed.
    On Windows Server 2003, Windows XP, and Windows 2000, the default setting for number of keep-alive probes is 5.
    The number of keep-alive probes is controllable through the TcpMaxDataRetransmissions and PPTPTcpMaxDataRetransmissions registry settings.
    The number of keep-alive probes is set to the larger of the two registry key values.
    If this number is 0, then keep-alive probes will not be sent.
    If this number is above 255, then it is adjusted to 255.
    */
    tcp_keepalive params;
    params.onoff = 1;
    params.keepalivetime = 30000; // entry is in milliseconds
    // set interval to target timeout divided by probe count
    params.keepaliveinterval = timeout * 1000 / 10;
    DWORD bytesReturned;
    if (WSAIoctl(handle, SIO_KEEPALIVE_VALS, &params, sizeof(params),
      0, 0, &bytesReturned, 0, 0)!= 0)
    {
      qiLogWarning() << "Failed to set socket keepalive with code " << WSAGetLastError();
    }
#else
    int optval = 1;
    unsigned int optlen = sizeof(optval);
    if(setsockopt(handle, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen) < 0)
      qiLogWarning() << "Failed to set so_keepalive: " << strerror(errno);
    else
    {
# if __linux__
      // Keepalive mechanism is overriden by TCP data retransmission mechanism
      // retransmission cannot be parameterized on a per-socket level

      /* SOL_TCP level options: unit is seconds for times
      TCP_KEEPCNT: overrides tcp_keepalive_probes 9
        mark dead when that many probes are lost
      TCP_KEEPIDLE: overrides tcp_keepalive_time 7200
        only enable keepalive if that delay ever occurr between data sent
      TCP_KEEPINTVL: overrides  tcp_keepalive_intvl 75
        interval between probes
      */
      optval = timeout / 10;
      if (setsockopt(handle, SOL_TCP, TCP_KEEPINTVL, &optval, optlen) < 0)
        qiLogWarning() << "Failed to set TCP_KEEPINTVL: " << strerror(errno);
      optval = 30;
      if (setsockopt(handle, SOL_TCP, TCP_KEEPIDLE , &optval, optlen) < 0)
        qiLogWarning() << "Failed to set TCP_KEEPIDLE : " << strerror(errno);
      optval = 10;
      if (setsockopt(handle, SOL_TCP, TCP_KEEPCNT  , &optval, optlen) < 0)
        qiLogWarning() << "Failed to set TCP_KEEPCNT  : " << strerror(errno);
     qiLogDebug() << "enabling TCP_USER_TIMEOUT";
     // this is messy: defined in linux/tcp.h only after kernel 2.6.37,
     // which conflicts with netinet/tcp.h
     // We do not want to rely on compile-time flag to enable/disable this,
     // so just try it.
     static bool tcpUserTimeoutWarning = false;
     static const int QI_TCP_USER_TIMEOUT = 18;
     // TCP_USER_TIMEOUT: maximum time in ms data can remain unaknowledged
     optval = timeout * 1000;
     if (setsockopt(handle, SOL_TCP, QI_TCP_USER_TIMEOUT, &optval, optlen) < 0
       && !tcpUserTimeoutWarning)
     {
        qiLogVerbose() << "(Expected on old kernels) Failed to set TCP_USER_TIMEOUT  : " << strerror(errno);
        tcpUserTimeoutWarning = true;
     }
# else
      // Macos only have TCP_KEEPALIVE wich is linux's TCP_KEEPIDLE
      // So best we can do is lower that, which will reduce delay from
      // hours to tens of minutes.
      optval = 30;
      if (setsockopt(handle, IPPROTO_TCP, TCP_KEEPALIVE , &optval, optlen) < 0)
        qiLogWarning() << "Failed to set TCP_KEEPALIVE : " << strerror(errno);
# endif
    }
#endif
  }

  FutureSync<void> TcpTransportSocket::disconnect()
  {
    boost::recursive_mutex::scoped_lock l(_mutex);
    if (_status == qi::TransportSocket::Status::Disconnected)
      return Future<void>{nullptr};

    qiLogDebug() << "Disconnecting TCP transport socket";
    error("Disconnection requested");
    return Future<void>{nullptr};
  }

  bool TcpTransportSocket::send(const qi::Message &msg)
  {
    boost::recursive_mutex::scoped_lock l(_mutex);
    if (!_socket || _status != qi::TransportSocket::Status::Connected)
    {
      qiLogDebug() << this << "Send on closed socket";
      return false;
    }

    qiLogDebug() << this << " Send (" << msg.type() << "):" << msg.address();
    if (!_sending)
    {
      _sending = true;
      send_(msg);
    }
    else
      _sendQueue.push_back(msg);
    return true;
  }

  void TcpTransportSocket::send_(qi::Message msg)
  {
    assert(_socket);
    using boost::asio::buffer;
    std::vector<boost::asio::const_buffer> b;
    msg._p->complete();
    // Send header
    b.push_back(buffer(msg._p->getHeader(), sizeof(qi::MessagePrivate::MessageHeader)));
    const qi::Buffer& buf = msg.buffer();
    size_t sz = buf.size();
    const std::vector<std::pair<size_t, Buffer> >& subs = buf.subBuffers();
    size_t pos = 0;
    // Handle subbuffers
    for (unsigned i=0; i< subs.size(); ++i)
    {
      // Send parent buffer between pos and start of sub
      size_t end = subs[i].first+4;
      if (end != pos)
        b.push_back(buffer((const char*)buf.data() + pos, end-pos));
      pos = end;
      // Send subbuffer
      b.push_back(buffer(subs[i].second.data(), subs[i].second.size()));
    }
    b.push_back(buffer((const char*)buf.data() + pos, sz - pos));

    _dispatcher.sent(msg);

    if (_ssl)
    {
      boost::asio::async_write(*_socket, b,
        boost::bind(&TcpTransportSocket::sendCont, shared_from_this(), _1, msg, _socket));
    }
    else
    {
      boost::asio::async_write(_socket->next_layer(), b,
        boost::bind(&TcpTransportSocket::sendCont, shared_from_this(), _1, msg, _socket));
    }
  }

  /*
   * warning: msg is given to the callback so as not to drop buffers refcount
   */
  void TcpTransportSocket::sendCont(const boost::system::error_code& erc, qi::Message, SocketPtr)
  {
    boost::recursive_mutex::scoped_lock l(_mutex);
    // The class does not wait for us to terminate, but it will set abort to true.
    // So do not use this before checking abort.
    if (erc || !_socket || _sendQueue.empty())
    { // errors are not processed here, since the read callback already does it
      _sending = false;
      return;
    }

    auto m = _sendQueue.front();
    _sendQueue.pop_front();
    send_(m);
  }
}
