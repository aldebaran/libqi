#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_TCPTRANSPORTSOCKET_P_HPP_
#define _SRC_TCPTRANSPORTSOCKET_P_HPP_

# include <string>
# include <queue>
# include <boost/thread/recursive_mutex.hpp>
# include <boost/asio.hpp>
# include <qi/atomic.hpp>
# include <qimessaging/api.hpp>
# include <qimessaging/message.hpp>
# include <qimessaging/url.hpp>
# include <qimessaging/transportsocket.hpp>
# include <qi/eventloop.hpp>

# include "transportsocket_p.hpp"


namespace qi
{

  class TcpTransportSocketPrivate : public TransportSocketPrivate
  {
  public:
    TcpTransportSocketPrivate(TransportSocket *self, EventLoop* eventloop = getDefaultNetworkEventLoop());
    TcpTransportSocketPrivate(TransportSocket *self, void* s, EventLoop* eventloop = getDefaultNetworkEventLoop());
    virtual ~TcpTransportSocketPrivate();

    virtual qi::FutureSync<bool> connect(const qi::Url &url);
    virtual qi::FutureSync<void> disconnect();
    virtual bool send(const qi::Message &msg);
    void startReading();

  private:
    void error(const boost::system::error_code& erc);
    void connected(const boost::system::error_code& erc);
    void onRead();
    void onReadHeader(const boost::system::error_code& erc, std::size_t);
    void onReadData(const boost::system::error_code& erc, std::size_t);

    void send_(qi::Message* msg);
    void sendCont(const boost::system::error_code& erc, Message* msg, boost::shared_ptr<bool> abort);
    void connect_(TransportSocketPtr socket, const qi::Url &url);
    void disconnect_(TransportSocketPtr socket);
  private:
    boost::asio::ip::tcp::socket& _socket;
    boost::shared_ptr<bool> _abort; // used to notify send callback sendCont that we are dead
    qi::Promise<bool>   _connectPromise;
    qi::Promise<void>   _disconnectPromise;

    // data to rebuild message
    bool                _readHdr;
    qi::Message        *_msg;
    bool                _connecting;

    boost::mutex        _sendQueueMutex; // protects _sendQueue, _sending and closing
    std::deque<Message> _sendQueue;
    bool                _sending;
  };

}

#endif  // _SRC_TCPTRANSPORTSOCKET_P_HPP_
