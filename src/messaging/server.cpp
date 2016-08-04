/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include <qi/anyobject.hpp>
#include "transportserver.hpp"
#include <qi/messaging/serviceinfo.hpp>
#include <qi/type/objecttypebuilder.hpp>
#include "objectregistrar.hpp"
#include <qi/os.hpp>
#include <boost/thread/mutex.hpp>
#include <exception>
#include "servicedirectoryclient.hpp"
#include "authprovider_p.hpp"

qiLogCategory("qimessaging.server");

namespace qi {

  //Server
  Server::Server(bool enforceAuth)
    : _enforceAuth(enforceAuth)
    , _dying(false)
    , _defaultCallType(qi::MetaCallType_Queued)
    , _newConnectionLink(_server.newConnection.connect(&Server::onTransportServerNewConnection, this, _1, true))
  {
  }

  Server::~Server()
  {
    //we can call reset on server and socket they are only owned by us.
    //when it's close it's close
    _server.newConnection.disconnect(_newConnectionLink);
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

  void Server::setAuthProviderFactory(AuthProviderFactoryPtr factory)
  {
    _authProviderFactory = factory;
  }

  namespace server_private
  {
    static void sendCapabilities(TransportSocketPtr sock)
    {
      Message msg;
      msg.setType(Message::Type_Capability);
      msg.setService(Message::Service_Server);
      msg.setValue(sock->localCapabilities(), typeOf<CapabilityMap>()->signature());
      sock->send(msg);
    }
  } // server_private

  void Server::connectMessageReady(const TransportSocketPtr& socket)
  {
    boost::recursive_mutex::scoped_lock sl(_socketsMutex);
    auto& subscriber = _subscribers[socket];

    QI_ASSERT(subscriber.messageReady == qi::SignalBase::invalidSignalLink &&
           "Connecting a signal that already exists.");

    subscriber.messageReady =
        socket->messageReady.connect(&Server::onMessageReady, this, _1, socket).setCallType(MetaCallType_Direct);
  }

  void Server::onTransportServerNewConnection(TransportSocketPtr socket, bool startReading)
  {
    boost::recursive_mutex::scoped_lock sl(_socketsMutex);
    if (!socket)
      return;
    if (_dying)
    {
      qiLogDebug() << "Incoming connection while closing, dropping...";
      socket->disconnect().async();
      return;
    }

    auto inserted = _subscribers.insert(std::make_pair(socket, SocketSubscriber{}));
    QI_ASSERT(inserted.second && "Socket insertion failed. Socket already exists.");

    auto& subscriber = inserted.first->second;

    QI_ASSERT(subscriber.disconnected == qi::SignalBase::invalidSignalLink && "Connecting a signal that already exists.");
    subscriber.disconnected =
        socket->disconnected.connect(&Server::onSocketDisconnected, this, socket, _1);

    // If false : the socket is only being registered, and has already been authenticated. The connection
    // was made elsewhere.
    // If true, it's an actual connection to this server.
    if (startReading)
    {
      auto signalLink = boost::make_shared<qi::SignalLink>();
      auto first = boost::make_shared<bool>(true);
      // We are reading on the socket for the first time : the first message has to be the capabilities
      *signalLink = socket->messageReady.connect(
            &Server::onMessageReadyNotAuthenticated, this, _1, socket,
            _authProviderFactory->newProvider(), first, signalLink)
          .setCallType(MetaCallType_Direct);

      socket->startReading();
    }
    else
    {
      QI_ASSERT(subscriber.messageReady == qi::SignalBase::invalidSignalLink &&
             "Connecting a signal that already exists.");
      subscriber.messageReady =
          socket->messageReady.connect(&Server::onMessageReady, this, _1, socket).setCallType(MetaCallType_Direct);
    }
  }

  void Server::onMessageReadyNotAuthenticated(const Message &msg, TransportSocketPtr socket, AuthProviderPtr auth,
                                              boost::shared_ptr<bool> first, boost::shared_ptr<qi::SignalLink> signalLink)
  {
    qiLogVerbose() << "Starting auth message" << msg.address();
    int id = msg.id();
    int service = msg.service();
    int function = msg.action();
    int type = msg.type();
    Message reply;

    reply.setId(id);
    reply.setService(service);
    if (service != Message::Service_Server
        || type != Message::Type_Call
        || function != Message::ServerFunction_Authenticate)
    {
      socket->messageReady.disconnect(*signalLink);
      if (_enforceAuth)
      {
        std::stringstream err;

        err << "Expected authentication (service #" << Message::Service_Server <<
               ", type #" << Message::Type_Call <<
               ", action #" << Message::ServerFunction_Authenticate <<
               "), received service #" << service << ", type #" << type << ", action #" << function;
        reply.setType(Message::Type_Error);
        reply.setError(err.str());
        socket->send(reply);
        socket->disconnect();
        qiLogVerbose() << err.str();
      }
      else
      {
        server_private::sendCapabilities(socket);
        qiLogVerbose() << "Authentication is not enforced. Skipping...";

        connectMessageReady(socket);
        onMessageReady(msg, socket);
      }
      return;
    }
    // the socket now contains the remote capabilities in socket->remoteCapabilities()
    qiLogVerbose() << "Authenticating client " << socket->remoteEndpoint().str() << "...";

    AnyReference cmref = msg.value(typeOf<CapabilityMap>()->signature(), socket);
    CapabilityMap authData = cmref.to<CapabilityMap>();
    cmref.destroy();
    CapabilityMap authResult = auth->processAuth(authData);
    unsigned int state = authResult[AuthProvider::State_Key].to<unsigned int>();
    std::string cmsig = typeOf<CapabilityMap>()->signature().toString();
    reply.setFunction(function);
    switch (state)
    {
    case AuthProvider::State_Done:
      qiLogVerbose() << "Client " << socket->remoteEndpoint().str() << " successfully authenticated.";
      socket->messageReady.disconnect(*signalLink);
      connectMessageReady(socket);
      // no break, we know that authentication is done, send the response to the remote end
    case AuthProvider::State_Cont:
      if (*first)
      {
        authResult.insert(socket->localCapabilities().begin(), socket->localCapabilities().end());
        *first = false;
      }
      reply.setValue(authResult, cmsig);
      reply.setType(Message::Type_Reply);
      socket->send(reply);
      break;
    case AuthProvider::State_Error:
    default:{
      std::stringstream builder;
      builder << "Authentication failed";
      if (authResult.find(AuthProvider::Error_Reason_Key) != authResult.end())
      {
        builder << ": " << authResult[AuthProvider::Error_Reason_Key].to<std::string>();
        builder << " [" << _authProviderFactory->authVersionMajor() << "." << _authProviderFactory->authVersionMinor() << "]";
      }
      reply.setType(Message::Type_Error);
      reply.setError(builder.str());
      qiLogVerbose() << builder.str();
      socket->send(reply);
      socket->disconnect();
      }
    }
    qiLogVerbose() << "Auth ends";
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
          || msg.type() == Message::Type_Error
          || msg.type() == Message::Type_Canceled)
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

  void Server::disconnectSignals(const TransportSocketPtr& socket, const SocketSubscriber& subscriber)
  {
    socket->connected.disconnectAll();
    socket->disconnected.disconnect(subscriber.disconnected);
    socket->messageReady.disconnect(subscriber.messageReady);
    socket->disconnect();
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
      const auto subscribersCopy = [&]
      {
        boost::recursive_mutex::scoped_lock sl(_socketsMutex);
        return std::move(_subscribers);
      }();

      for (auto& pair : subscribersCopy)
        disconnectSignals(pair.first, pair.second);
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
        // Lock the mutex, erase the socket, and disconnect it outside the lock.
        auto socketLocal = [&]()
        {
          boost::recursive_mutex::scoped_lock sl(_socketsMutex);
          auto it = _subscribers.find(socket);
          QI_ASSERT(it != _subscribers.end());
          auto local = std::move(*it);
          _subscribers.erase(it);
          return local;
        }();

        if (socketLocal.first)
          disconnectSignals(socketLocal.first, socketLocal.second);
      }
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
