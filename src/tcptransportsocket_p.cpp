/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include <iostream>
#include <cstring>
#include <map>

#ifdef ANDROID
#include <linux/in.h> // for  IPPROTO_TCP
#endif

#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "tcptransportsocket.hpp"
#include "transportsocket_p.hpp"
#include "tcptransportsocket_p.hpp"
#include "message_p.hpp"

#include <qi/log.hpp>
#include <qi/types.hpp>
#include <qimessaging/session.hpp>
#include <qimessaging/message.hpp>
#include <qi/buffer.hpp>
#include <qi/eventloop.hpp>

#define MAX_LINE 16384

namespace qi
{

  //#### TransportSocketLibEvent


  TcpTransportSocketPrivate::TcpTransportSocketPrivate(TransportSocket *self, EventLoop* eventLoop)
    : TransportSocketPrivate(self, eventLoop)
    , _socket(*new boost::asio::ip::tcp::socket(*(boost::asio::io_service*)eventLoop->nativeHandle()))
    , _readHdr(true)
    , _msg(0)
    , _connecting(false)
    , _sending(false)
  {
    assert(eventLoop);
    _disconnectPromise.setValue(0); // not connected, so we are finished disconnecting
  }

  TcpTransportSocketPrivate::TcpTransportSocketPrivate(TransportSocket *self,
    void* s, EventLoop* eventLoop)
    : TransportSocketPrivate(self, eventLoop)
    , _socket(*(boost::asio::ip::tcp::socket*)s)
    , _readHdr(true)
    , _msg(0)
    , _connecting(false)
    , _sending(false)
  {
    assert(eventLoop);
    _status = qi::TransportSocket::Status_Connected;
  }



  TcpTransportSocketPrivate::~TcpTransportSocketPrivate()
  {
    disconnect();
    delete _msg;
    delete &_socket;
    qiLogVerbose("qi.tcpsocket") << "deleted " << this;
  }

  void TcpTransportSocketPrivate::startReading()
  {
    _msg = new qi::Message();
    boost::asio::async_read(_socket,
      boost::asio::buffer(_msg->_p->getHeader(), sizeof(MessagePrivate::MessageHeader)),
      boost::bind(&TcpTransportSocketPrivate::onReadHeader, this, _1, _2));
  }

  void TcpTransportSocketPrivate::onReadHeader(const boost::system::error_code& erc,
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
      boost::asio::async_read(_socket,
        boost::asio::buffer(ptr, payload),
        boost::bind(&TcpTransportSocketPrivate::onReadData, this, _1, _2));
    }
    else
      onReadData(boost::system::error_code(), 0);
  }

  void TcpTransportSocketPrivate::onReadData(const boost::system::error_code& erc,
    std::size_t len)
  {
    if (erc)
    {
      error(erc);
      return;
    }
      // qiLogDebug("TransportSocket") << "Recv (" << _msg->type() << "):" << _msg->address();
    static int usWarnThreshold = os::getenv("QIMESSAGING_SOCKET_DISPATCH_TIME_WARN_THRESHOLD").empty()?0:strtol(os::getenv("QIMESSAGING_SOCKET_DISPATCH_TIME_WARN_THRESHOLD").c_str(),0,0);
    qi::int64_t start = 0;
    if (usWarnThreshold)
      start = os::ustime(); // call might be not that cheap
    _self->messageReady(*_msg);
    _dispatcher.dispatch(*_msg);
    if (usWarnThreshold)
    {
      qi::int64_t duration = os::ustime() - start;
      if (duration > usWarnThreshold)
        qiLogWarning("TransportSocket") << "Dispatch to user took " << duration << "us";
    }
    delete _msg;
    startReading();
  }

  void TcpTransportSocketPrivate::error(const boost::system::error_code& erc)
  {
    _status = qi::TransportSocket::Status_Disconnected;
    _self->disconnected(erc.value());

    if (_connecting)
    {
      _connecting = false;
      _connectPromise.setError(std::string("Connection error: ") + erc.message());
    }

    {
      boost::mutex::scoped_lock l(_sendQueueMutex);
      boost::system::error_code er;
      if (_socket.is_open())
        _socket.close(er);
    }

    _disconnectPromise.setValue(0);
  }


  qi::FutureSync<bool> TcpTransportSocketPrivate::connect(const qi::Url &url)
  {
    if (_status == qi::TransportSocket::Status_Connected || _connecting)
    {
      qiLogError("qimessaging.TransportSocketLibevent") << "connection already in progress";
      return makeFutureError<bool>("Operation already in progress");
    }
    _url = url;
    _connectPromise.reset();
    _disconnectPromise.reset();
    _status = qi::TransportSocket::Status_Connecting;
    _connecting = true;
    _err = 0;
    if (_url.port() == 0) {
      qiLogError("qimessaging.TransportSocket") << "Error try to connect to a bad address: " << _url.str();
      _connectPromise.setError("Bad address " + _url.str());
      _status = qi::TransportSocket::Status_Disconnected;
      _connecting = false;
      _disconnectPromise.setValue(0);
      return _connectPromise.future();
    }
    qiLogVerbose("qimessaging.transportsocket.connect") << "Trying to connect to " << _url.host() << ":" << _url.port();
    using namespace boost::asio;
    // Resolve url
    ip::tcp::resolver r(_socket.get_io_service());
    ip::tcp::resolver::query q(_url.host(), boost::lexical_cast<std::string>(_url.port()));
    // Synchronous resolution

    try
    {
      ip::tcp::resolver::iterator it = r.resolve(q);
      // asynchronous connect
      _socket.async_connect(*it,
        boost::bind(&TcpTransportSocketPrivate::connected, this, _1));
      return _connectPromise.future();
    }
    catch (const std::exception& e)
    {
      qiLogError("qimessaging.transportsocket.connect") << e.what();
      _connectPromise.setValue(false);
      return _connectPromise.future();
    }
  }

  void TcpTransportSocketPrivate::connected(const boost::system::error_code& erc)
  {
    _connecting = false;
    if (erc)
    {
      qiLogWarning("qimessaging.TransportSocketLibEvent") << "connect: " << erc.message();
      _status = qi::TransportSocket::Status_Disconnected;
      _connectPromise.setValue(false);
      _disconnectPromise.setValue(0);
      //FIXME: ?? _connectPromise.setError(erc.message());
    }
    else
    {
      _status = qi::TransportSocket::Status_Connected;
      _connectPromise.setValue(true);
      _self->connected();
      startReading();
    }
  }

  qi::FutureSync<void> TcpTransportSocketPrivate::disconnect()
  {
    {
      boost::mutex::scoped_lock l(_sendQueueMutex);
      if (_socket.is_open())
      {
        boost::system::error_code erc;
        _socket.close(erc); // will invoke read callback with error set
      }
      // Do not set disconnectPromise here, it will/has been set
      // by error(), called by read callback, and we must wait for it
      // to terminate.
    }
    return _disconnectPromise.future();
  }

  bool TcpTransportSocketPrivate::send(const qi::Message &msg)
  {
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

  void TcpTransportSocketPrivate::send_(qi::Message* msg)
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
    boost::asio::async_write(_socket, b,
      boost::bind(&TcpTransportSocketPrivate::sendCont, this, _1, msg));
  }

  void TcpTransportSocketPrivate::sendCont(const boost::system::error_code& erc,
    Message* msg)
  {
    delete msg;
    if (erc)
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

