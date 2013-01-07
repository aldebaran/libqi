#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_SERVER_HPP_
#define _SRC_SERVER_HPP_

#include <string>
#include <set>
#include <boost/thread/recursive_mutex.hpp>
#include <qimessaging/api.hpp>
#include <qimessaging/session.hpp>
#include <qimessaging/transportserver.hpp>
#include <qi/atomic.hpp>
#include "boundobject.hpp"

namespace qi {

  /**
   * Do all the plumbing between sockets and objects.
   *
   * Support a special kind of objects: (SocketObject, that are aware of Socket)
   *
   * Threadsafety warning: do not call listen and addSocketObject at the same time
   *
   */
  class Server {
  public:
    QI_DISALLOW_COPY_AND_ASSIGN(Server);
    Server(qi::MetaCallType defaultCallType = qi::MetaCallType_Queued);
    ~Server();

    //make the server listening
    bool listen(const qi::Url &address);
    void close();
    qi::Url listenUrl() const;

    //Create a BoundObject
    bool addObject(unsigned int idx, qi::ObjectPtr obj);
    bool addObject(unsigned int idx, qi::BoundObjectPtr obj);
    bool removeObject(unsigned int idx);

    qi::UrlVector endpoints() const;

    void setDefaultCallType(qi::MetaCallType ctype);

  private:
    void setSocketObjectEndpoints();

  private:
    //TransportServer
    void onTransportServerNewConnection(TransportSocketPtr socket);

    //TransportSocket
    void onSocketDisconnected(TransportSocketPtr socket, int error);
    void onMessageReady(const qi::Message &msg, TransportSocketPtr socket);

  private:
    //bool: true if it's a socketobject
    typedef std::map<unsigned int, BoundObjectPtr> BoundObjectPtrMap;

    //ObjectList
    BoundObjectPtrMap                   _boundObjects;
    boost::mutex                        _boundObjectsMutex;

    //SocketList
    std::set<TransportSocketPtr>        _sockets;
    boost::recursive_mutex              _socketsMutex;

  public:
    TransportServer                     _server;
    bool                                _dying;
    qi::MetaCallType                    _defaultCallType;
  };
}


#endif  // _SRC_SESSIONSERVER_HPP_
