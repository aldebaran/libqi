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

    virtual bool connect(qi::Session *session,
                         const qi::Url &url)  = 0;
    virtual void disconnect()                 = 0;
    virtual bool send(const qi::Message &msg) = 0;
    virtual bool read(int id, qi::Message *msg);

    // if msecs < 0 no timeout
    virtual bool waitForConnected(int msecs = 30000);
    virtual bool waitForDisconnected(int msecs = 30000);
    virtual bool waitForId(int id, int msecs = 30000);

    virtual void setCallbacks(TransportSocketInterface *delegate);
    virtual bool isConnected() const;

  public:
    TransportSocketInterface *tcd;
    bool                      connected;
    int                       status;
    // data to rebuild message
    bool                      readHdr;
    qi::Message              *msg;
    qi::TransportSocket      *self;

    std::map<unsigned int, qi::Message*> msgSend;
    boost::mutex                         mtx;
    boost::condition_variable            cond;
    qi::Url                              url;
  private:
    TransportSocketPrivate() {}
  };
}
#endif  // _QIMESSAGING_TRANSPORT_SOCKET_P_HPP_
