#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QIMESSAGING_TRANSPORTSERVER_HPP_
#define _QIMESSAGING_TRANSPORTSERVER_HPP_

# include <qimessaging/api.hpp>
# include <qimessaging/url.hpp>
# include <qi/eventloop.hpp>
# include <qitype/signal.hpp>
# include <vector>


namespace qi {

  class TransportSocket;
  class TransportServer;
  class Session;
  typedef boost::shared_ptr<TransportSocket> TransportSocketPtr;
  class TransportServerPrivate;

  class TransportServer
  {
    QI_DISALLOW_COPY_AND_ASSIGN(TransportServer);

  public:
    TransportServer();
    virtual ~TransportServer();

    qi::Future<void> listen(const qi::Url &url,
                            qi::EventLoop* ctx = qi::getDefaultNetworkEventLoop());
    bool setIdentity(const std::string& key, const std::string& crt);
    void close();

    std::vector<qi::Url> endpoints() const;

  public:
    /** Emited each time a new connection happens. startReading must be
     * called on the socket
     */
    // C4251
    qi::Signal<void (TransportSocketPtr)> newConnection;
    // C4251
    qi::Signal<void (int error)>          acceptError;
    qi::Signal<void (void)>               endpointsChanged;

  public:
    TransportServerPrivate *_p;
  };

}

#endif  // _QIMESSAGING_TRANSPORTSERVER_HPP_
