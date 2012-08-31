/*
** Author(s):
**  - Herve Cuche <hcuche@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#pragma once
#ifndef _QIMESSAGING_TRANSPORT_SOCKET_P_HPP_
# define _QIMESSAGING_TRANSPORT_SOCKET_P_HPP_

# include <string>
# include <vector>
# include <boost/thread.hpp>

# include <qimessaging/api.hpp>
# include <qimessaging/message.hpp>
# include <qimessaging/url.hpp>
# include <qimessaging/transport_socket.hpp>


namespace qi
{
  class Session;
  class TransportSocketPrivate
  {
  public:
    TransportSocketPrivate(TransportSocket *socket);
    virtual ~TransportSocketPrivate();

    virtual bool connect(const qi::Url &url, EventLoop* ctx)  = 0;
    virtual void disconnect()                 = 0;
    virtual bool send(const qi::Message &msg) = 0;
    virtual bool read(int id, qi::Message *msg);

    // if msecs < 0 no timeout
    virtual bool waitForConnected(int msecs = 30000);
    virtual bool waitForDisconnected(int msecs = 30000);
    virtual bool waitForId(int id, int msecs = 30000);

    virtual void addCallbacks(TransportSocketInterface *delegate, void *data);
    virtual void removeCallbacks(TransportSocketInterface *delegate);
    virtual bool isConnected() const;
    virtual void destroy() = 0;
  public:

    typedef std::vector< std::pair<TransportSocketInterface *, void *> > TransportSocketInterfaceVector;
    TransportSocketInterfaceVector          tcd;
    boost::recursive_mutex                  mtxCallback;
    bool                                    connected;
    int                                     status;
    // data to rebuild message
    bool                      readHdr;
    qi::Message              *msg;
    qi::TransportSocket      *self;

    struct PendingMessage
    {
      PendingMessage() : msg(0), timestamp(0) {}
      qi::Message         *msg;
      time_t               timestamp;
    };

    std::map<unsigned int, PendingMessage> msgSend;
    boost::mutex                         mtx;
    boost::condition_variable            cond;
    qi::Url                              url;
  private:
    TransportSocketPrivate() {}
  };
}
#endif  // _QIMESSAGING_TRANSPORT_SOCKET_P_HPP_
