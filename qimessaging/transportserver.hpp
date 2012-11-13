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
# include <qimessaging/url.hpp>

#ifdef _MSC_VER
#  pragma warning( push )
#  pragma warning( disable: 4251 )
#endif

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
    /** Emited each time a new connection happens. startReading must be
     * called on the socket
     */
    // C4251
    qi::Signal<void (TransportSocketPtr)> newConnection;
    // C4251
    qi::Signal<void (int error)>          acceptError;

  public:
    TransportServerPrivate *_p;
  };

}

#ifdef _MSC_VER
#  pragma warning( pop )
#endif

#endif  // _QIMESSAGING_TRANSPORTSERVER_HPP_
