/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include <set>
#include <qitype/genericobject.hpp>
#include <qimessaging/transportserver.hpp>
#include <qimessaging/serviceinfo.hpp>
#include <qitype/objecttypebuilder.hpp>
#include "objectregistrar.hpp"
#include "serverresult.hpp"
#include "transportserver_p.hpp"
#include <qi/os.hpp>
#include <boost/thread/mutex.hpp>
#include "servicedirectoryclient.hpp"

namespace qi {

  //Server
  Server::Server(qi::MetaCallType defaultCallType)
    : _boundObjectsMutex()
    , _server()
    , _dying(false)
    , _defaultCallType(defaultCallType)
  {
    _server.newConnection.connect(boost::bind<void>(&Server::onTransportServerNewConnection, this, _1));
  }

  Server::~Server()
  {
    //we can call reset on server and socket they are only owned by us.
    //when it's close it's close
    _server.newConnection.disconnectAll();
    boost::recursive_mutex::scoped_lock sl(_socketsMutex);
    _dying = true;
    for (std::set<TransportSocketPtr>::iterator i = _sockets.begin();
      i != _sockets.end(); ++i)
    {
      // We do not want onSocketDisconnected called
      //TODO: move that logic into TransportServer.
      (*i)->disconnected.disconnectAll();
      (*i)->messageReady.disconnectAll();
      (*i)->disconnect();
    }
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

  void Server::setDefaultCallType(qi::MetaCallType ctype) {
    _defaultCallType = ctype;
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
    _sockets.insert(socket);
    socket->disconnected.connect(boost::bind<void>(&Server::onSocketDisconnected, this, socket, _1));
    socket->messageReady.connect(boost::bind<void>(&Server::onMessageReady, this, _1, socket));
    socket->startReading();
  }


  qi::Url Server::listenUrl() const {
    return _server.listenUrl();
  }

  void Server::onMessageReady(const qi::Message &msg, TransportSocketPtr socket) {
    qi::BoundObjectPtr obj;
    // qiLogDebug("Server") << "Server Recv (" << msg.type() << "):" << msg.address();

    {
      boost::mutex::scoped_lock sl(_boundObjectsMutex);
      BoundObjectPtrMap::iterator it;

      it = _boundObjects.find(msg.service());
      if (it == _boundObjects.end())
      {
        if (msg.type() == qi::Message::Type_Call) {
          qi::Message       retval(Message::Type_Error, msg.address());
          qi::Buffer        error;
          qi::BinaryEncoder ds(error);
          std::stringstream ss;
          ss << "can't find service, address: " << msg.address();
          ds.write(qi::typeOf<std::string>()->signature());
          ds.write(ss.str());
          retval.setBuffer(error);
          socket->send(retval);
        }
        qiLogError("qi::Server") << "Can't find service: " << msg.service();
        return;
      }
      obj            = it->second;
    }
    qi::getDefaultObjectEventLoop()->asyncCall(0, boost::bind<void>(&BoundObject::onMessage, obj, msg, socket));
    //obj->onMessage(msg, socket);
  }

  void Server::close()
  {
    if (listenUrl().str() == "")
    {
      return;
    }

    qiLogInfo("Server") << "Closing server: " << listenUrl().str();
    {
      boost::recursive_mutex::scoped_lock sl(_socketsMutex);
      std::set<TransportSocketPtr>::iterator it;
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

  bool Server::listen(const qi::Url &address)
  {
    qi::Url url(address);

    if (url.protocol() != "tcp") {
      qiLogError("qi::Server") << "Protocol " << url.protocol() << " not supported.";
      return false;
    }
    if (!_server.listen(url))
      return false;

    qiLogVerbose("qimessaging.Server") << "Started Server at " << _server.listenUrl().str();
    return true;
  }

  void Server::onSocketDisconnected(TransportSocketPtr socket, int error)
  {
    BoundObjectPtrMap::iterator it;
    {
      boost::mutex::scoped_lock sl(_boundObjectsMutex);
      for (it = _boundObjects.begin(); it != _boundObjects.end(); ++it) {
        BoundObjectPtr o = it->second;
        o->onSocketDisconnected(socket, error);
      }
    }
  }

  qi::UrlVector Server::endpoints() const {
    return _server.endpoints();
  }


}
