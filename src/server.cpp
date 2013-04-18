/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include <qitype/genericobject.hpp>
#include "transportserver.hpp"
#include <qimessaging/serviceinfo.hpp>
#include <qitype/objecttypebuilder.hpp>
#include "objectregistrar.hpp"
#include "serverresult.hpp"
#include <qi/os.hpp>
#include <boost/thread/mutex.hpp>
#include "servicedirectoryclient.hpp"

qiLogCategory("qimessaging.server");

namespace qi {

  //Server
  Server::Server()
    : _boundObjectsMutex()
    , _server()
    , _dying(false)
    , _defaultCallType(qi::MetaCallType_Queued)
  {
    _server.newConnection.connect(boost::bind<void>(&Server::onTransportServerNewConnection, this, _1));
  }

  Server::~Server()
  {
    close();
  }

  bool Server::addObject(unsigned int id, qi::ObjectPtr obj)
  {
    if (!obj)
      return false;
    BoundObjectPtr bop = makeServiceBoundObjectPtr(id, obj, _defaultCallType);
    return addObject(id, bop);
  }

  bool Server::addObject(unsigned int id, qi::BoundObjectPtr obj)
  {
    if (!obj)
      return false;
    //register into _boundObjects
    {
      boost::mutex::scoped_lock sl(_boundObjectsMutex);
      BoundObjectPtrMap::iterator it;
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
      BoundObjectPtrMap::iterator it;
      it = _boundObjects.find(idx);
      if (it == _boundObjects.end()) {
        return false;
      }
      _boundObjects.erase(idx);
    }
    return true;
  }

  void Server::onTransportServerNewConnection(TransportSocketPtr socket)
  {
    boost::recursive_mutex::scoped_lock sl(_socketsMutex);
    if (!socket)
      return;
    _sockets.push_back(socket);
    socket->disconnected.connect(boost::bind<void>(&Server::onSocketDisconnected, this, socket, _1));
    socket->messageReady.connect(boost::bind<void>(&Server::onMessageReady, this, _1, socket));
    socket->startReading();
  }

  void Server::onMessageReady(const qi::Message &msg, TransportSocketPtr socket) {
    qi::BoundObjectPtr obj;
    // qiLogDebug() << "Server Recv (" << msg.type() << "):" << msg.address();

    {
      boost::mutex::scoped_lock sl(_boundObjectsMutex);
      BoundObjectPtrMap::iterator it;

      it = _boundObjects.find(msg.service());
      if (it == _boundObjects.end())
      {
        qi::Message       retval(Message::Type_Error, msg.address());
        std::stringstream ss;
        ss << "can't find service, address: " << msg.address();
        retval.setError(ss.str());
        socket->send(retval);
        qiLogError() << "Can't find service: " << msg.service();
        return;
      }
      obj            = it->second;
    }
    qi::getDefaultThreadPoolEventLoop()->post(boost::bind<void>(&BoundObject::onMessage, obj, msg, socket));
    //obj->onMessage(msg, socket);
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

    //we can call reset on server and socket they are only owned by us.
    //when it's close it's close
    _server.newConnection.disconnectAll();

    qiLogInfo() << "Closing server...";
    {
      boost::recursive_mutex::scoped_lock sl(_socketsMutex);
      std::list<TransportSocketPtr>::iterator it;
      //TODO move that logic into TransportServer
      for (it = _sockets.begin(); it != _sockets.end(); ++it) {
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
    return _server.listen(address);
  }

  bool Server::setIdentity(const std::string& key, const std::string& crt)
  {
    return _server.setIdentity(key, crt);
  }

  void Server::onSocketDisconnected(TransportSocketPtr socket, int error)
  {
    boost::mutex::scoped_lock l(_stateMutex);
    if (_dying)
    {
      return;
    }

    BoundObjectPtrMap::iterator it;
    {
      boost::mutex::scoped_lock sl(_boundObjectsMutex);
      for (it = _boundObjects.begin(); it != _boundObjects.end(); ++it) {
        BoundObjectPtr o = it->second;
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
      std::list<TransportSocketPtr>::iterator it;
      boost::recursive_mutex::scoped_lock sl(_socketsMutex);
      for (it = _sockets.begin(); it != _sockets.end(); ++it)
      {
        if (it->get() == socket.get())
        {
          (*it)->connected.disconnectAll();
          (*it)->disconnected.disconnectAll();
          (*it)->messageReady.disconnectAll();
          _sockets.erase(it);
          break;
        }
      }
    }
  }

  qi::UrlVector Server::endpoints() const {
    return _server.endpoints();
  }


}
