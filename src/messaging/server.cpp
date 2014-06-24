/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include <qi/anyobject.hpp>
#include "transportserver.hpp"
#include <qi/messaging/serviceinfo.hpp>
#include <qi/type/objecttypebuilder.hpp>
#include "objectregistrar.hpp"
#include "serverresult.hpp"
#include <qi/os.hpp>
#include <boost/thread/mutex.hpp>
#include "servicedirectoryclient.hpp"

qiLogCategory("qimessaging.server");

namespace qi {

  //Server
  Server::Server()
    : qi::Trackable<Server>(this)
    ,_boundObjectsMutex()
    , _server()
    , _dying(false)
    , _defaultCallType(qi::MetaCallType_Queued)
  {
    _server.newConnection.connect(&Server::onTransportServerNewConnection, this, _1, true);
  }

  Server::~Server()
  {
    //we can call reset on server and socket they are only owned by us.
    //when it's close it's close
    _server.newConnection.disconnectAll();
    close();
    destroy();
  }

  bool Server::addObject(unsigned int id, qi::AnyObject obj)
  {
    if (!obj)
      return false;
    BoundAnyObject bop = makeServiceBoundAnyObject(id, obj, _defaultCallType);
    return addObject(id, bop);
  }

  bool Server::addObject(unsigned int id, qi::BoundAnyObject obj)
  {
    if (!obj)
      return false;
    //register into _boundObjects
    {
      boost::mutex::scoped_lock sl(_boundObjectsMutex);
      BoundAnyObjectMap::iterator it;
      it = _boundObjects.find(id);
      if (it != _boundObjects.end()) {
        return false;
      }
      _boundObjects[id] = obj;
      return true;
    }
  }


  bool Server::removeObject(unsigned int idx)
  {
    {
      boost::mutex::scoped_lock sl(_boundObjectsMutex);
      BoundAnyObjectMap::iterator it;
      it = _boundObjects.find(idx);
      if (it == _boundObjects.end()) {
        return false;
      }
      _boundObjects.erase(idx);
    }
    return true;
  }

  void Server::onTransportServerNewConnection(TransportSocketPtr socket, bool startReading)
  {
    boost::recursive_mutex::scoped_lock sl(_socketsMutex);
    if (!socket)
      return;
    if (_dying)
    {
      qiLogDebug() << "Incoming connectiong while closing, dropping...";
      socket->disconnect().async();
      return;
    }
    _sockets.push_back(socket);
    socket->disconnected.connect(&Server::onSocketDisconnected, this, socket, _1);
    socket->messageReady.connect(&Server::onMessageReady, this, _1, socket);
    if (startReading)
      socket->startReading();
  }

  void Server::onMessageReady(const qi::Message &msg, TransportSocketPtr socket) {
    qi::BoundAnyObject obj;
    // qiLogDebug() << "Server Recv (" << msg.type() << "):" << msg.address();
    {
      boost::mutex::scoped_lock sl(_boundObjectsMutex);
      BoundAnyObjectMap::iterator it;

      it = _boundObjects.find(msg.service());
      if (it == _boundObjects.end())
      {
        // The message could be addressed to a bound object, inside a
        // remoteobject host, or to a remoteobject, using the same socket.
        qiLogVerbose() << "No service for " << msg.address();
        if (msg.object() > Message::GenericObject_Main
          || msg.type() == Message::Type_Reply
          || msg.type() == Message::Type_Event
          || msg.type() == Message::Type_Error)
          return;
        // ... but only if the object id is >main
        qi::Message       retval(Message::Type_Error, msg.address());
        std::stringstream ss;
        ss << "can't find service, address: " << msg.address();
        retval.setError(ss.str());
        socket->send(retval);
        qiLogError() << "Can't find service: " << msg.service() << " on " << msg.address();
        return;
      }
      obj            = it->second;
    }
    // We were called from the thread pool: synchronous call is ok
    //qi::getEventLoop()->post(boost::bind<void>(&BoundObject::onMessage, obj, msg, socket));
    obj->onMessage(msg, socket);
  }

  void Server::close()
  {
    {
      boost::mutex::scoped_lock l(_stateMutex);

      if (_dying)
      {
        return;
      }

      _dying = true;
    }

    qiLogVerbose() << "Closing server...";
    {
      std::list<TransportSocketPtr> socketsCopy;
      {
        boost::recursive_mutex::scoped_lock sl(_socketsMutex);
        std::swap(_sockets, socketsCopy);
      }
      std::list<TransportSocketPtr>::iterator it;
      //TODO move that logic into TransportServer
      for (it = socketsCopy.begin(); it != socketsCopy.end(); ++it) {
        (*it)->connected.disconnectAll();
        (*it)->disconnected.disconnectAll();
        (*it)->messageReady.disconnectAll();
        (*it)->disconnect();
      }
    }
    _server.close();
  }

  qi::Future<void> Server::listen(const qi::Url &address)
  {
    // Assume this means we are back on-line.
    _dying = false;
    return _server.listen(address);
  }

  bool Server::setIdentity(const std::string& key, const std::string& crt)
  {
    return _server.setIdentity(key, crt);
  }

  void Server::onSocketDisconnected(TransportSocketPtr socket, std::string error)
  {
    TransportSocketPtr match;
    {
      boost::mutex::scoped_lock l(_stateMutex);
      if (_dying)
      {
        return;
      }

      BoundAnyObjectMap::iterator it;
      {
        boost::mutex::scoped_lock sl(_boundObjectsMutex);
        for (it = _boundObjects.begin(); it != _boundObjects.end(); ++it) {
          BoundAnyObject o = it->second;
          try
          {
            o->onSocketDisconnected(socket, error);
          }
          catch (const std::runtime_error& e)
          {
            qiLogError() << e.what();
          }
        }
      }

      {
        {
          boost::recursive_mutex::scoped_lock sl(_socketsMutex);
          std::list<TransportSocketPtr>::iterator it = std::find(_sockets.begin(), _sockets.end(), socket);
          if (it != _sockets.end())
          {
            match = *it;
            _sockets.erase(it);
          }
        }
      }
    }
    if (match)
    {
      match->connected.disconnectAll();
      match->disconnected.disconnectAll();
      match->messageReady.disconnectAll();
    }
  }

  qi::UrlVector Server::endpoints() const {
    return _server.endpoints();
  }

  void Server::open()
  {
    _dying = false;
  }


}
