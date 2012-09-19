#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QIMESSAGING_TRANSPORTSERVER_HPP_
#define _QIMESSAGING_TRANSPORTSERVER_HPP_

# include <qimessaging/api.hpp>
# include <qimessaging/url.hpp>
# include <qimessaging/eventloop.hpp>
# include <qimessaging/signal.hpp>
# include <vector>
# include <qimessaging/url.hpp>

namespace qi {

  class TransportSocket;
  class TransportServer;
  class Session;
  typedef boost::shared_ptr<TransportSocket> TransportSocketPtr;
  class TransportServerPrivate;

  class QIMESSAGING_API TransportServer
  {
    QI_DISALLOW_COPY_AND_ASSIGN(TransportServer);

  public:
    TransportServer();
    TransportServer(const qi::Url &url,
                    qi::EventLoop* ctx = qi::getDefaultNetworkEventLoop());
    virtual ~TransportServer();


    bool listen();
    bool listen(const qi::Url &url,
                qi::EventLoop* ctx = qi::getDefaultNetworkEventLoop());
    void close();

    qi::Url listenUrl() const;
    std::vector<qi::Url> endpoints() const;

  public:
    qi::Signal<void (TransportSocketPtr)> newConnection;
    qi::Signal<void (int error)>          acceptError;

  public:
    TransportServerPrivate *_p;
  };

}

#endif  // _QIMESSAGING_TRANSPORTSERVER_HPP_
