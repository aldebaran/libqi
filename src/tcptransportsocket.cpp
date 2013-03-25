/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

// Disable "'this': used in base member initializer list"
#ifdef _MSC_VER
# pragma warning( push )
# pragma warning(disable: 4355)
#endif

#include <iostream>
#include <cstring>
#include <map>

#ifdef ANDROID
#include <linux/in.h> // for  IPPROTO_TCP
#endif

#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/make_shared.hpp>

#include "tcptransportsocket.hpp"

#include <qi/log.hpp>
#include <qi/types.hpp>
#include <qimessaging/session.hpp>
#include "message.hpp"
#include <qi/buffer.hpp>
#include <qi/eventloop.hpp>

#define MAX_LINE 16384

qiLogCategory("qimessaging.transportsocket");

namespace qi
{
  TcpTransportSocket::TcpTransportSocket(EventLoop* eventLoop, bool ssl, void* s)
    : TransportSocket()
    , _ssl(ssl)
    , _sslHandshake(false)
#ifdef WITH_SSL
    , _sslContext(boost::asio::ssl::context::sslv23)
#endif
    , _abort(boost::make_shared<bool>(false))
    , _readHdr(true)
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
      _socket = (boost::asio::ssl::stream<boost::asio::ip::tcp::socket>*) s;
#else
      _socket = (boost::asio::ip::tcp::socket*) s;
#endif
      _status = qi::TransportSocket::Status_Connected;

      // Transmit each Message without delay
      const boost::asio::ip::tcp::no_delay option( true );
      _socket->lowest_layer().set_option(option);
    }
    else
    {
#ifdef WITH_SSL
      if (_ssl)
      {
        _sslContext.set_verify_mode(boost::asio::ssl::verify_none);
      }

      _socket = (new boost::asio::ssl::stream<boost::asio::ip::tcp::socket>((*(boost::asio::io_service*)eventLoop->nativeHandle()), _sslContext));
#else
      _socket = new boost::asio::ip::tcp::socket(*(boost::asio::io_service*)eventLoop->nativeHandle());
#endif

      _disconnectPromise.setValue(0); // not connected, so we are finished disconnecting
    }
  }



  TcpTransportSocket::~TcpTransportSocket()
  {
    disconnect();
    delete _msg;
    delete _socket;
    qiLogVerbose() << "deleted " << this;
  }

  void TcpTransportSocket::startReading()
  {
    _msg = new qi::Message();

    boost::mutex::scoped_lock(_closingMutex);

    if (*_abort == true)
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
          boost::bind(&TcpTransportSocket::handshake, this, _1));
        return;
      }

      boost::asio::async_read(*_socket,
        boost::asio::buffer(_msg->_p->getHeader(), sizeof(MessagePrivate::MessageHeader)),
        boost::bind(&TcpTransportSocket::onReadHeader, this, _1, _2));
    }
    else
    {
      boost::asio::async_read(_socket->next_layer(),
        boost::asio::buffer(_msg->_p->getHeader(), sizeof(MessagePrivate::MessageHeader)),
        boost::bind(&TcpTransportSocket::onReadHeader, this, _1, _2));
    }
#else
    boost::asio::async_read(*_socket,
      boost::asio::buffer(_msg->_p->getHeader(), sizeof(MessagePrivate::MessageHeader)),
      boost::bind(&TcpTransportSocket::onReadHeader, this, _1, _2));
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

      boost::mutex::scoped_lock(_closingMutex);

      if (*_abort == true)
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
          boost::bind(&TcpTransportSocket::onReadData, this, _1, _2));
      }
      else
      {
        boost::asio::async_read(_socket->next_layer(),
          boost::asio::buffer(ptr, payload),
          boost::bind(&TcpTransportSocket::onReadData, this, _1, _2));
      }
#else
      boost::asio::async_read(*_socket,
        boost::asio::buffer(ptr, payload),
        boost::bind(&TcpTransportSocket::onReadData, this, _1, _2));
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
    _status = qi::TransportSocket::Status_Disconnected;
    disconnected(erc.value());

    if (_connecting)
    {
      _connecting = false;
      _connectPromise.setError(std::string("Connection error: ") + erc.message());
    }

    {
      boost::mutex::scoped_lock l(_sendQueueMutex);
      boost::system::error_code er;
      if (_socket->lowest_layer().is_open())
        _socket->lowest_layer().close(er);
    }

    _disconnectPromise.setValue(0);
  }


  qi::FutureSync<void> TcpTransportSocket::connect(const qi::Url &url)
  {
    if (_status == qi::TransportSocket::Status_Connected || _connecting)
    {
      const char* s = "connection already in progress";
      qiLogError() << s;
      return makeFutureError<void>(s);
    }
    _url = url;
    _connectPromise.reset();
    _disconnectPromise.reset();
    _status = qi::TransportSocket::Status_Connecting;
    _connecting = true;
    _err = 0;
    if (_url.port() == 0) {
      qiLogError() << "Error try to connect to a bad address: " << _url.str();
      _connectPromise.setError("Bad address " + _url.str());
      _status = qi::TransportSocket::Status_Disconnected;
      _connecting = false;
      _disconnectPromise.setValue(0);
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
        boost::bind(&TcpTransportSocket::connected2, this, _1));
      return _connectPromise.future();
    }
    catch (const std::exception& e)
    {
      const char* s = e.what();
      qiLogError() << s;
      _connectPromise.setError(s);
      return _connectPromise.future();
    }
  }

  void TcpTransportSocket::handshake(const boost::system::error_code& erc)
  {
    if (erc)
    {
      qiLogWarning() << "connect: " << erc.message();
      _status = qi::TransportSocket::Status_Disconnected;
      _connectPromise.setError(erc.message());
      _disconnectPromise.setValue(0);
      //FIXME: ?? _connectPromise.setError(erc.message());
    }
    else
    {
      _status = qi::TransportSocket::Status_Connected;
      _connectPromise.setValue(0);
      connected();
      _sslHandshake = true;
      // Transmit each Message without delay
      const boost::asio::ip::tcp::no_delay option( true );
      _socket->lowest_layer().set_option(option);
      startReading();
    }
  }

  void TcpTransportSocket::connected2(const boost::system::error_code& erc)
  {
    _connecting = false;
    if (erc)
    {
      qiLogWarning("qimessaging.TransportSocketLibEvent") << "connect: " << erc.message();
      _status = qi::TransportSocket::Status_Disconnected;
      _connectPromise.setError(erc.message());
      _disconnectPromise.setValue(0);
    }
    else
    {
      if (_ssl)
      {
#ifdef WITH_SSL
        _socket->async_handshake(boost::asio::ssl::stream_base::client,
          boost::bind(&TcpTransportSocket::handshake, this, _1));
#endif
      }
      else
      {
        _status = qi::TransportSocket::Status_Connected;
        _connectPromise.setValue(0);
        connected();
        // Transmit each Message without delay
        const boost::asio::ip::tcp::no_delay option( true );
        _socket->lowest_layer().set_option(option);
        startReading();
      }
    }
  }

  qi::FutureSync<void> TcpTransportSocket::disconnect()
  {
    *_abort = true; // Notify send callback sendCont that it must silently terminate
    {
      boost::mutex::scoped_lock l(_sendQueueMutex);
      if (_socket->lowest_layer().is_open())
      {
        boost::system::error_code erc;
        boost::mutex::scoped_lock(_closingMutex);

        _socket->lowest_layer().shutdown(boost::asio::ip::tcp::socket::shutdown_both, erc);
        _socket->lowest_layer().close(erc); // will invoke read callback with error set
      }
      // Do not set disconnectPromise here, it will/has been set
      // by error(), called by read callback, and we must wait for it
      // to terminate.
    }
    return _disconnectPromise.future();
  }

  bool TcpTransportSocket::send(const qi::Message &msg)
  {
    qiLogDebug() << this << " Send (" << msg.type() << "):" << msg.address();
    boost::mutex::scoped_lock lock(_sendQueueMutex);

    if (!_sending)
    {
      _sending = true;
      send_(new Message(msg));
    }
    else
      _sendQueue.push_back(msg);
    return true;
  }

  void TcpTransportSocket::send_(qi::Message* msg)
  {
    using boost::asio::buffer;
    std::vector<boost::asio::const_buffer> b;
    msg->_p->complete();
    // Send header
    b.push_back(buffer(msg->_p->getHeader(), sizeof(qi::MessagePrivate::MessageHeader)));
    const qi::Buffer& buf = msg->buffer();
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
    _dispatcher.sent(*msg);

#ifdef WITH_SSL
    if (_ssl)
    {
      boost::asio::async_write(*_socket, b,
        boost::bind(&TcpTransportSocket::sendCont, this, _1, msg, _abort));
    }
    else
    {
      boost::asio::async_write(_socket->next_layer(), b,
        boost::bind(&TcpTransportSocket::sendCont, this, _1, msg, _abort));
    }
#else
    boost::asio::async_write(*_socket, b,
      boost::bind(&TcpTransportSocket::sendCont, this, _1, msg, _abort));
#endif
  }

  void TcpTransportSocket::sendCont(const boost::system::error_code& erc,
    Message* msg, boost::shared_ptr<bool> abort)
  {
    delete msg;
    // The class does not wait for us to terminate, but it will set abort to true.
    // So do not use this before checking abort.
    if (erc || *abort)
      return; // read-callback will also get the error, avoid dup and ignore it
    boost::mutex::scoped_lock lock(_sendQueueMutex);
    if (_sendQueue.empty())
      _sending = false;
    else
    {
      msg = new Message(_sendQueue.front());
      _sendQueue.pop_front();
      send_(msg);
    }
  }

}

