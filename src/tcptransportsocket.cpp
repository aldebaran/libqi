/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

// Disable "'this': used in base member initializer list"
#ifdef _MSC_VER
# pragma warning( push )
# pragma warning(disable: 4355)
#endif

#ifdef ANDROID
#include <linux/in.h> // for  IPPROTO_TCP
#endif

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/make_shared.hpp>

#include "tcptransportsocket.hpp"

#include <qi/log.hpp>

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
  TcpTransportSocket::TcpTransportSocket(EventLoop* eventLoop, bool ssl, void* s)
    : TransportSocket()
    , _ssl(ssl)
    , _sslHandshake(false)
#ifdef WITH_SSL
    , _sslContext(boost::asio::ssl::context::sslv23)
#endif
    , _abort(false)
    , _msg(0)
    , _connecting(false)
    , _sending(false)
  {
    _eventLoop = eventLoop;
    _err = 0;
    _status = qi::TransportSocket::Status_Disconnected;

    if (s != 0)
    {
#ifdef WITH_SSL
      _socket = SocketPtr((boost::asio::ssl::stream<boost::asio::ip::tcp::socket>*) s);
#else
      _socket = SocketPtr((boost::asio::ip::tcp::socket*) s);
#endif
      _status = qi::TransportSocket::Status_Connected;

      // Transmit each Message without delay
      const boost::asio::ip::tcp::no_delay option( true );
      _socket->lowest_layer().set_option(option);
    }
    else
    {
      pSetValue(_disconnectPromise); // not connected, so we are finished disconnecting
    }
  }



  TcpTransportSocket::~TcpTransportSocket()
  {
    qiLogDebug() << this;
    boost::system::error_code erc;
    error(erc);
    delete _msg;
    qiLogVerbose() << "deleted " << this;
  }

  void TcpTransportSocket::startReading()
  {
    qiLogDebug() << this;

    _msg = new qi::Message();

    boost::recursive_mutex::scoped_lock l(_closingMutex);

    if (_abort)
    {
      boost::system::error_code erc;
      error(erc);
      return;
    }

#ifdef WITH_SSL
    if (_ssl)
    {
      if (!_sslHandshake)
      {
        _socket->async_handshake(boost::asio::ssl::stream_base::server,
          boost::bind(&TcpTransportSocket::handshake, shared_from_this(), _1));
        return;
      }

      boost::asio::async_read(*_socket,
        boost::asio::buffer(_msg->_p->getHeader(), sizeof(MessagePrivate::MessageHeader)),
        boost::bind(&TcpTransportSocket::onReadHeader, shared_from_this(), _1, _2));
    }
    else
    {
      boost::asio::async_read(_socket->next_layer(),
        boost::asio::buffer(_msg->_p->getHeader(), sizeof(MessagePrivate::MessageHeader)),
        boost::bind(&TcpTransportSocket::onReadHeader, shared_from_this(), _1, _2));
    }
#else
    boost::asio::async_read(*_socket,
      boost::asio::buffer(_msg->_p->getHeader(), sizeof(MessagePrivate::MessageHeader)),
      boost::bind(&TcpTransportSocket::onReadHeader, shared_from_this(), _1, _2));
#endif
  }

  void TcpTransportSocket::onReadHeader(const boost::system::error_code& erc,
    std::size_t len)
  {
    if (erc)
    {
      error(erc);
      return;
    }
    size_t payload = _msg->_p->header.size;
    if (payload)
    {
      void* ptr = _msg->_p->buffer.reserve(payload);

      boost::recursive_mutex::scoped_lock l(_closingMutex);

      if (_abort)
      {
        boost::system::error_code erc;
        error(erc);
        return;
      }

#ifdef WITH_SSL
      if (_ssl)
      {
        boost::asio::async_read(*_socket,
          boost::asio::buffer(ptr, payload),
          boost::bind(&TcpTransportSocket::onReadData, shared_from_this(), _1, _2));
      }
      else
      {
        boost::asio::async_read(_socket->next_layer(),
          boost::asio::buffer(ptr, payload),
          boost::bind(&TcpTransportSocket::onReadData, shared_from_this(), _1, _2));
      }
#else
      boost::asio::async_read(*_socket,
        boost::asio::buffer(ptr, payload),
        boost::bind(&TcpTransportSocket::onReadData, shared_from_this(), _1, _2));
#endif
    }
    else
      onReadData(boost::system::error_code(), 0);
  }

  void TcpTransportSocket::onReadData(const boost::system::error_code& erc,
    std::size_t len)
  {
    if (erc)
    {
      error(erc);
      return;
    }
      qiLogDebug() << this << " Recv (" << _msg->type() << "):" << _msg->address();
    static int usWarnThreshold = os::getenv("QIMESSAGING_SOCKET_DISPATCH_TIME_WARN_THRESHOLD").empty()?0:strtol(os::getenv("QIMESSAGING_SOCKET_DISPATCH_TIME_WARN_THRESHOLD").c_str(),0,0);
    qi::int64_t start = 0;
    if (usWarnThreshold)
      start = os::ustime(); // call might be not that cheap
    messageReady(*_msg);
    _dispatcher.dispatch(*_msg);
    if (usWarnThreshold)
    {
      qi::int64_t duration = os::ustime() - start;
      if (duration > usWarnThreshold)
        qiLogWarning() << "Dispatch to user took " << duration << "us";
    }
    delete _msg;
    startReading();
  }

  void TcpTransportSocket::error(const boost::system::error_code& erc)
  {
    qiLogDebug() << "socket error:" << erc.message();
    boost::recursive_mutex::scoped_lock lock(_closingMutex);
    _abort = true;
    _status = qi::TransportSocket::Status_Disconnected;
    disconnected(erc.message());

    if (_connecting)
    {
      _connecting = false;
      pSetError(_connectPromise, std::string("Connection error: ") + erc.message());
    }

    {
      boost::mutex::scoped_lock l(_sendQueueMutex);
      boost::system::error_code er;
      if (_socket && _socket->lowest_layer().is_open())
      {
        _socket->lowest_layer().shutdown(boost::asio::ip::tcp::socket::shutdown_both, er);
        _socket->lowest_layer().close(er);

      }
    }
    _socket.reset();
    pSetValue(_disconnectPromise);
  }


  qi::FutureSync<void> TcpTransportSocket::connect(const qi::Url &url)
  {
    boost::recursive_mutex::scoped_lock l(_closingMutex);

    if (_status == qi::TransportSocket::Status_Connected || _connecting)
    {
      const char* s = "connection already in progress";
      qiLogError() << s;
      return makeFutureError<void>(s);
    }
#ifdef WITH_SSL
    if (_ssl)
    {
      _sslContext.set_verify_mode(boost::asio::ssl::verify_none);
    }
    _socket = SocketPtr(new boost::asio::ssl::stream<boost::asio::ip::tcp::socket>((*(boost::asio::io_service*)_eventLoop->nativeHandle()), _sslContext));
#else
    _socket = SocketPtr(new boost::asio::ip::tcp::socket(*(boost::asio::io_service*)_eventLoop->nativeHandle()));
#endif
    _url = url;
    _connectPromise.reset();
    _disconnectPromise.reset();
    _status = qi::TransportSocket::Status_Connecting;
    _connecting = true;
    _err = 0;
    if (_url.port() == 0) {
      qiLogError() << "Error try to connect to a bad address: " << _url.str();
      pSetError(_connectPromise, std::string("Bad address ") + _url.str());

      _status = qi::TransportSocket::Status_Disconnected;
      _connecting = false;
      pSetValue(_disconnectPromise);
      return _connectPromise.future();
    }
    qiLogVerbose() << "Trying to connect to " << _url.host() << ":" << _url.port();
    using namespace boost::asio;
    // Resolve url
    ip::tcp::resolver r(_socket->get_io_service());
    ip::tcp::resolver::query q(_url.host(), boost::lexical_cast<std::string>(_url.port()));
    // Synchronous resolution

    try
    {
      ip::tcp::resolver::iterator it = r.resolve(q);
      // asynchronous connect
      _socket->lowest_layer().async_connect(*it,
        boost::bind(&TcpTransportSocket::onConnected, shared_from_this(), _1));
      return _connectPromise.future();
    }
    catch (const std::exception& e)
    {
      const char* s = e.what();
      qiLogError() << s;
      pSetError(_connectPromise, s);
      return _connectPromise.future();
    }
  }

  void TcpTransportSocket::handshake(const boost::system::error_code& erc)
  {
    if (erc)
    {
      qiLogWarning() << "connect: " << erc.message();
      _status = qi::TransportSocket::Status_Disconnected;
      pSetError(_connectPromise, erc.message());
      pSetValue(_disconnectPromise);
    }
    else
    {
      _status = qi::TransportSocket::Status_Connected;
      pSetValue(_connectPromise);
      connected();
      _sslHandshake = true;

      {
        boost::recursive_mutex::scoped_lock l(_closingMutex);

        if (_abort)
        {
          return;
        }
        // Transmit each Message without delay
        const boost::asio::ip::tcp::no_delay option( true );
        _socket->lowest_layer().set_option(option);
      }

      startReading();
    }
  }

  void TcpTransportSocket::onConnected(const boost::system::error_code& erc)
  {
    _connecting = false;
    if (erc)
    {
      qiLogWarning("qimessaging.TransportSocketLibEvent") << "connect: " << erc.message();
      _status = qi::TransportSocket::Status_Disconnected;
      pSetError(_connectPromise, erc.message());
      pSetValue(_disconnectPromise);
    }
    else
    {
      if (_ssl)
      {
#ifdef WITH_SSL
        _socket->async_handshake(boost::asio::ssl::stream_base::client,
          boost::bind(&TcpTransportSocket::handshake, shared_from_this(), _1));
#endif
      }
      else
      {
        _status = qi::TransportSocket::Status_Connected;
        pSetValue(_connectPromise);
        connected();

        {
          boost::recursive_mutex::scoped_lock l(_closingMutex);

          if (_abort)
          {
            return;
          }
          // Transmit each Message without delay
          const boost::asio::ip::tcp::no_delay option( true );
          _socket->lowest_layer().set_option(option);
        }

        startReading();
      }
    }
  }

  qi::FutureSync<void> TcpTransportSocket::disconnect()
  {
    boost::system::error_code erc;
    _eventLoop->post(boost::bind(&TcpTransportSocket::error,
                                 boost::static_pointer_cast<TcpTransportSocket>(shared_from_this()),
                                 erc));
    return _disconnectPromise.future();
  }

  bool TcpTransportSocket::send(const qi::Message &msg)
  {
    // Check that once before locking in case some idiot tries to send
    // from a disconnect notification.
    if (_status != qi::TransportSocket::Status_Connected)
      return false;
    boost::recursive_mutex::scoped_lock lockc(_closingMutex);

    if (!_socket || _status != qi::TransportSocket::Status_Connected)
    {
      qiLogDebug() << this << "Send on closed socket";
      return false;
    }

    qiLogDebug() << this << " Send (" << msg.type() << "):" << msg.address();
    boost::mutex::scoped_lock lock(_sendQueueMutex);

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

    boost::recursive_mutex::scoped_lock l(_closingMutex);

    if (_abort)
    {
      qiLogWarning() << "send aborted";
      return;
    }

    _dispatcher.sent(msg);

#ifdef WITH_SSL
    if (_ssl)
    {
      boost::asio::async_write(*_socket, b,
        boost::bind(&TcpTransportSocket::sendCont, shared_from_this(), _1, msg));
    }
    else
    {
      boost::asio::async_write(_socket->next_layer(), b,
        boost::bind(&TcpTransportSocket::sendCont, shared_from_this(), _1, msg));
    }
#else
    boost::asio::async_write(*_socket, b,
      boost::bind(&TcpTransportSocket::sendCont, shared_from_this(), _1, msg));
#endif
  }

  /*
   * warning: msg is given to the callback so as not to drop buffers refcount
   */
  void TcpTransportSocket::sendCont(const boost::system::error_code& erc, qi::Message msg)
  {
    // The class does not wait for us to terminate, but it will set abort to true.
    // So do not use this before checking abort.
    if (erc || _abort)
      return; // read-callback will also get the error, avoid dup and ignore it

    qi::Message m;
    {
      boost::mutex::scoped_lock lock(_sendQueueMutex);
      if (_sendQueue.empty())
      {
        _sending = false;
        return;
      }

      m = _sendQueue.front();
      _sendQueue.pop_front();
    }

    send_(m);
  }

}

