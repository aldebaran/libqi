#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_SERVER_HPP_
#define _SRC_SERVER_HPP_

#include <boost/thread/recursive_mutex.hpp>
#include <qimessaging/api.hpp>
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
    Server();
    ~Server();

    //make the server listening
    qi::Future<void> listen(const qi::Url &address);
    void close();
    bool setIdentity(const std::string& key, const std::string& crt);

    //Create a BoundObject
    bool addObject(unsigned int idx, qi::ObjectPtr obj);
    bool addObject(unsigned int idx, qi::BoundObjectPtr obj);
    bool removeObject(unsigned int idx);

    std::vector<qi::Url> endpoints() const;


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
    std::list<TransportSocketPtr>       _sockets;
    boost::recursive_mutex              _socketsMutex;
    boost::mutex                        _stateMutex;

  public:
    TransportServer                     _server;
    bool                                _dying;
    qi::MetaCallType                    _defaultCallType;
  };
}


#endif  // _SRC_SESSIONSERVER_HPP_
