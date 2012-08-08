/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010, 2012 Aldebaran Robotics
*/

#include <set>
#include <qimessaging/object.hpp>
#include <qimessaging/server.hpp>
#include <qimessaging/functor.hpp>
#include <qimessaging/transport_server.hpp>
#include <qimessaging/service_info.hpp>
#include "src/network_thread.hpp"
#include "src/session_p.hpp"
#include "src/server_functor_result_future_p.hpp"
#include "src/transport_server_p.hpp"
#include <qi/os.hpp>
#include <boost/thread/mutex.hpp>

namespace qi {

  class ServerPrivate : public TransportServerInterface,
                        public TransportSocketInterface,
                        public FutureInterface<unsigned int>
  {
  public:

    ServerPrivate();
    virtual ~ServerPrivate();
    virtual void newConnection(TransportSocket *client);
    virtual void onSocketReadyRead(TransportSocket *client, int id);
    virtual void onSocketDisconnected(TransportSocket *client);
    virtual void onFutureFinished(const unsigned int &future,
                                  void *data);
    virtual void onFutureFailed(const std::string &error, void *data);
    bool         setSuitableEndpoints(const qi::Url &url);
  public:
    // (service, linkId)
    struct RemoteLink
    {
      RemoteLink()
        : localLinkId(0)
        , event(0)
      {}
      RemoteLink(unsigned int localLinkId, unsigned int event)
      : localLinkId(localLinkId)
      , event(event) {}
      unsigned int localLinkId;
      unsigned int event;
    };
    // remote link id -> local link id
    typedef std::map<unsigned int, RemoteLink> ServiceLinks;
    // service id -> links
    typedef std::map<unsigned int, ServiceLinks> PerServiceLinks;
    // socket -> all links
    // Keep track of links setup by clients to disconnect when the client exits.
    typedef std::map<TransportSocket*, PerServiceLinks > Links;
    Links                                   _links;

    std::set<TransportSocket*>              _clients;
    std::map<unsigned int, qi::Object*>     _services;
    std::map<std::string, qi::Object*>      _servicesByName;
    std::map<std::string, qi::ServiceInfo>  _servicesInfo;
    std::map<qi::Object*, qi::ServiceInfo>  _servicesObject;
    std::map<unsigned int, std::string>     _servicesIndex;
    TransportServer                        *_ts;
    std::vector<std::string>                _endpoints;
    qi::Session                            *_session;
    boost::mutex                            _mutexServices;
    boost::mutex                            _mutexOthers;
  };

  ServerPrivate::ServerPrivate()
    : _ts(new TransportServer()),
      _session(0)
  {
    _ts->addCallbacks(this);
  }

  ServerPrivate::~ServerPrivate() {
    for (std::set<TransportSocket*>::iterator i = _clients.begin();
      i != _clients.end(); ++i)
    {
      // We do not want onSocketDisconnected called
      (*i)->removeCallbacks(this);
      delete *i;
    }
    delete _ts;
  }

  void ServerPrivate::newConnection(TransportSocket *socket)
  {
    if (!socket)
      return;
    _clients.insert(socket);
    socket->addCallbacks(this);
  }

  void ServerPrivate::onSocketDisconnected(TransportSocket* client)
  {
    _clients.erase(client);
    // Disconnect event links set for this client.
    Links::iterator i = _links.find(client);
    if (i != _links.end())
    {
      // Iterate per service
      for (PerServiceLinks::iterator j = i->second.begin();
        j != i->second.end(); ++j)
      {
        std::map<unsigned int, qi::Object*>::iterator iservice = _services.find(j->first);
        // If the service is still there, disconnect one by one.
        if (iservice != _services.end())
          for (ServiceLinks::iterator k = j->second.begin();
            k != j->second.end(); ++k)
            iservice->second->disconnect(k->second.localLinkId);
      }
      _links.erase(i);
    }
    delete client;
  }

  // Grah give me boost::bind!
  class EventForwarder: public Functor
  {
  public:
    EventForwarder(unsigned int service, unsigned int event,
      TransportSocket* client)
    : _service(service)
    , _event(event)
    , _client(client)
    {}
    virtual void call(const qi::FunctorParameters &params, qi::FunctorResult result) const
    {
      qi::Message msg;
      msg.setBuffer(params.buffer());
      msg.setService(_service);
      msg.setFunction(_event);
      msg.setType(Message::Type_Event);
      msg.setPath(Message::Path_Main);
      _client->send(msg);
    }
  private:
    unsigned int _service;
    unsigned int _event;
    TransportSocket* _client;
  };
  void ServerPrivate::onSocketReadyRead(TransportSocket *client, int id) {
    qi::Message msg;
    client->read(id, &msg);
    qi::Object *obj;

    {
      boost::mutex::scoped_lock sl(_mutexServices);
      std::map<unsigned int, qi::Object*>::iterator it;
      if (msg.service() == Message::Service_Server)
      {
        // Accept register/unregister event as emit or as call
        if (msg.type() != Message::Type_Event
          && msg.type() != Message::Type_Call)
        {
          qiLogError("qi::Server") << "Server service only handles call/emit";
          qi::Message retval;
          retval.buildReplyFrom(msg);
          Buffer buf;
          ODataStream(buf) << "Server service only handles call/emit";
          retval.setBuffer(buf);
          client->send(retval);
          return;
        }
        // First arg is always a service id, so factor a bi there
        IDataStream ds(msg.buffer());
        unsigned int service;
        ds >> service;
        it = _services.find(service);
        if (it == _services.end())
        {
          if (msg.type() == Message::Type_Call)
          {
            qi::Message retval;
            retval.buildReplyFrom(msg);
            Buffer buf;
            ODataStream(buf) << "Service not found";
            retval.setBuffer(buf);
            client->send(retval);
          }
          return;
        }
        switch(msg.function())
        {

        case Message::ServerFunction_RegisterEvent:
          {
            unsigned int event, remoteLinkId;
            ds >> event >> remoteLinkId;

            // locate object, register locally and bounce to an event message
            unsigned int linkId = it->second->connect(event,
              new EventForwarder(service, event, client));
            _links[client][service][remoteLinkId] = RemoteLink(linkId, event);
            if (msg.type() == Message::Type_Call)
            {
              qi::Message retval;
              retval.buildReplyFrom(msg);
              Buffer buf;
              ODataStream ds(buf);
              ds << linkId;
              retval.setBuffer(buf);
              client->send(retval);
            }
          }
          break;
        case Message::ServerFunction_UnregisterEvent:
          {
            unsigned int event, remoteLinkId;
            ds >> event >> remoteLinkId;
            ServiceLinks& sl = _links[client][service];
            ServiceLinks::iterator i = sl.find(remoteLinkId);
            if (i == sl.end())
            {
              qiLogError("qi::Server") << "Unregister request failed for "
              << remoteLinkId <<" " << service;
            }
            else
            {
              it->second->disconnect(i->second.localLinkId);
            }
            if (msg.type() == Message::Type_Call)
            {
              qi::Message retval;
              retval.buildReplyFrom(msg);
              Buffer buf;
              ODataStream ds(buf);
              ds << (i == sl.end());
              retval.setBuffer(buf);
              client->send(retval);
            }
          }
          break;
        }
        return;
      } // msg.service() == Server
      it = _services.find(msg.service());
      obj = it->second;
      if (it == _services.end() || !obj)
      {
        if (msg.type() == qi::Message::Type_Call) {
          qi::Message retval;
          retval.buildReplyFrom(msg);
          qi::Buffer error;
          qi::ODataStream ds(error);
          std::stringstream ss;
          ss << "can't find service id: " << id;
          ds << ss.str();
          retval.setBuffer(error);
          client->send(retval);

        }
        qiLogError("qi::Server") << "Can't find service: " << msg.service();
        return;
      }
    }
    switch (msg.type())
    {
    case Message::Type_Call:
      {
         qi::FunctorParameters ds(msg.buffer());
         ServerFunctorResult promise(client, msg);
         obj->metaCall(msg.function(), ds, promise, qi::Object::MetaCallType_Queued);
      }
      break;
    case Message::Type_Event:
      {
        qi::FunctorParameters ds(msg.buffer());
        obj->metaEmit(msg.function(), ds);
      }
      break;
    }

  };

  void ServerPrivate::onFutureFailed(const std::string &error, void *data)
  {
    qi::ServiceInfo si;
    qi::Object     *obj = static_cast<qi::Object *>(data);

    std::map<qi::Object *, qi::ServiceInfo>::iterator it = _servicesObject.find(obj);
    if (it != _servicesObject.end())
      _servicesObject.erase(it);
  }

  void ServerPrivate::onFutureFinished(const unsigned int &idx,
                                       void               *data)
  {
    qi::Object     *obj = static_cast<qi::Object *>(data);
    qi::ServiceInfo si;
    std::map<qi::Object *, qi::ServiceInfo>::iterator it;

    {
      boost::mutex::scoped_lock sl(_mutexOthers);
      it = _servicesObject.find(obj);
      if (it != _servicesObject.end())
        si = _servicesObject[obj];
    }
    si.setServiceId(idx);

    {
      boost::mutex::scoped_lock sl(_mutexServices);
      _services[idx] = obj;
    }
    // ack the Service directory to tell that we are ready
    _session->_p->serviceReady(idx);
    {
      boost::mutex::scoped_lock sl(_mutexOthers);
      _servicesInfo[si.name()] = si;
      _servicesByName[si.name()] = obj;
      _servicesIndex[idx] = si.name();
      _servicesObject.erase(it);
    }
  }

  Server::Server()
    : _p(new ServerPrivate())
  {
  }

  Server::~Server()
  {
    delete _p;
  }

  bool ServerPrivate::setSuitableEndpoints(const qi::Url &url)
  {
    std::string protocol;
    std::map<std::string, std::vector<std::string> > ifsMap;

    _endpoints.clear();

    if (url.host().compare("0.0.0.0") != 0) {
      _endpoints.push_back(url.str());
      return true;
    }

    ifsMap = qi::os::hostIPAddrs();
    if (ifsMap.empty())
    {
      qiLogWarning("qimessaging.server.listen") << "Cannot get host addresses";
      return false;
    }
#ifdef WIN32 // hostIPAddrs doesn't return loopback on windows
    ifsMap["Loopback"].push_back("127.0.0.1");
#endif

    if (url.protocol() == "any")
      protocol = "tcp";
    else
      protocol = url.protocol();
    protocol += "://";

    for (std::map<std::string, std::vector<std::string> >::iterator interfaceIt = ifsMap.begin();
         interfaceIt != ifsMap.end();
         ++interfaceIt)
    {
      for (std::vector<std::string>::iterator addressIt = (*interfaceIt).second.begin();
           addressIt != (*interfaceIt).second.end();
           ++addressIt)
      {
        std::stringstream ss;
        ss << protocol;
        ss << (*addressIt);
        ss << ":";
        ss << url.port();
        qiLogVerbose("qimessaging.server.listen") << "Adding endpoint : " << ss.str();
        _endpoints.push_back(ss.str());
       }
    }
    return true;
  }

  bool Server::listen(qi::Session *session, const std::string &address)
  {
    qi::Url url(address);
    _p->_session = session;

    if (url.protocol() != "tcp") {
      qiLogError("qi::Server") << "Protocol " << url.protocol() << " not supported.";
      return false;
    }
    if (!_p->_ts->listen(session, url))
      return false;
    if (!_p->setSuitableEndpoints(_p->_ts->listenUrl())) {
      //TODO: cleanup...
      return false;
    }
    qiLogVerbose("qimessaging.Server") << "Started Server at " << _p->_ts->listenUrl().str();
    return true;
  }


  qi::Future<unsigned int> Server::registerService(const std::string &name,
                                                   qi::Object        *obj)
  {
    if (!_p->_session)
    {
      qiLogError("qimessaging.Server") << "no session attached to the server.";
      return qi::Future<unsigned int>();
    }

    if (_p->_endpoints.empty()) {
      qiLogError("qimessaging.Server") << "Could not register service: " << name << " because the current server has not endpoint";
      return qi::Future<unsigned int>();
    }
    qi::ServiceInfo si;
    si.setName(name);
    si.setProcessId(qi::os::getpid());
    si.setMachineId("TODO");
    si.setEndpoints(_p->_endpoints);

    {
      boost::mutex::scoped_lock sl(_p->_mutexOthers);
      _p->_servicesObject[obj] = si;
    }

    qi::Future<unsigned int> future;
    future.addCallbacks(_p, obj);
    future = _p->_session->_p->registerService(si, future);

    return future;
  };

  qi::Future<void> Server::unregisterService(unsigned int idx)
  {
    if (!_p->_session)
    {
      qiLogError("qimessaging.Server") << "no session attached to the server.";
      return qi::Future<void>();
    }

    qi::Future<void> future = _p->_session->_p->unregisterService(idx);

    {
      boost::mutex::scoped_lock sl(_p->_mutexServices);
      _p->_services.erase(idx);
    }
    {
      boost::mutex::scoped_lock sl(_p->_mutexOthers);
      std::map<unsigned int, std::string>::iterator it;
      it = _p->_servicesIndex.find(idx);
      if (it == _p->_servicesIndex.end()) {
        qiLogError("qimessaging.Server") << "Can't find name associated to id:" << idx;
      }
      else {
        _p->_servicesByName.erase(it->second);
        _p->_servicesInfo.erase(it->second);
      }
      _p->_servicesIndex.erase(idx);
    }
    return future;
  };

  void Server::close() {
    _p->_ts->close();
  }

  std::vector<qi::ServiceInfo> Server::registeredServices() {
    std::vector<qi::ServiceInfo> ssi;
    std::map<std::string, qi::ServiceInfo>::iterator it;

    {
      boost::mutex::scoped_lock sl(_p->_mutexOthers);
      for (it = _p->_servicesInfo.begin(); it != _p->_servicesInfo.end(); ++it) {
        ssi.push_back(it->second);
      }
    }
    return ssi;
  }

  qi::ServiceInfo Server::registeredService(const std::string &service) {
    std::map<std::string, qi::ServiceInfo>::iterator it;
    {
      boost::mutex::scoped_lock sl(_p->_mutexOthers);
      it = _p->_servicesInfo.find(service);
      if (it != _p->_servicesInfo.end())
        return it->second;
    }
    return qi::ServiceInfo();
  }

  qi::Object *Server::registeredServiceObject(const std::string &service) {
    std::map<std::string, qi::Object *>::iterator it;
    {
      boost::mutex::scoped_lock sl(_p->_mutexOthers);
      it = _p->_servicesByName.find(service);
      if (it != _p->_servicesByName.end())
        return it->second;
    }
    return 0;
  }

  qi::Url Server::listenUrl() const {
    return _p->_ts->listenUrl();
  }

}
