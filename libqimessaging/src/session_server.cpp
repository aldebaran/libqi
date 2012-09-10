/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010, 2012 Aldebaran Robotics
*/

#include <set>
#include <qimessaging/object.hpp>
#include <qimessaging/transport_server.hpp>
#include <qimessaging/service_info.hpp>
#include "session_server.hpp"
#include "src/server_result.hpp"
#include "src/transport_server_p.hpp"
#include <qi/os.hpp>
#include <boost/thread/mutex.hpp>
#include "service_directory_client.hpp"

namespace qi {

  static MetaFunctionResult forwardEvent(const MetaFunctionParameters& params,
                                         unsigned int service, unsigned int event, TransportSocket* client)
  {
    qi::Message msg;
    msg.setBuffer(params.getBuffer());
    msg.setService(service);
    msg.setFunction(event);
    msg.setType(Message::Type_Event);
    msg.setObject(Message::Object_Main);
    client->send(msg);
    return MetaFunctionResult();
  }


  Session_Server::Session_Server(ServiceDirectoryClient *sdClient)
    : _server()
    , _dying(false)
    , _sdClient(sdClient)
  {
    _server.addCallbacks(this);
  }

  Session_Server::~Session_Server()
  {
    _dying = true;
    boost::recursive_mutex::scoped_lock sl(_mutexOthers);
    for (std::set<TransportSocket*>::iterator i = _clients.begin();
      i != _clients.end(); ++i)
    {
      // We do not want onSocketDisconnected called
      (*i)->removeCallbacks(this);
      delete *i;
    }
  }


  void Session_Server::onTransportServerNewConnection(TransportServer* server, TransportSocket *socket, void *data)
  {
    boost::recursive_mutex::scoped_lock sl(_mutexOthers);
    if (!socket)
      return;
    _clients.insert(socket);
    socket->addCallbacks(this);
  }

  void Session_Server::onSocketDisconnected(TransportSocket* client, void *data)
  {
    // The check below must be done before holding the lock.
    if (_dying)
      return;
    boost::recursive_mutex::scoped_lock sl(_mutexOthers);
    _clients.erase(client);
    // Disconnect event links set for this client.
    Links::iterator i = _links.find(client);
    if (i != _links.end())
    {
      // Iterate per service
      for (PerServiceLinks::iterator j = i->second.begin();
        j != i->second.end(); ++j)
      {
        std::map<unsigned int, qi::Object>::iterator iservice = _services.find(j->first);
        // If the service is still there, disconnect one by one.
        if (iservice != _services.end())
          for (ServiceLinks::iterator k = j->second.begin();
            k != j->second.end(); ++k)
            iservice->second.disconnect(k->second.localLinkId);
      }
      _links.erase(i);
    }
    delete client;
  }



  void Session_Server::onSocketReadyRead(TransportSocket *client, int id, void *data) {
    qi::Message msg;
    client->read(id, &msg);
    qi::Object obj;

    {
      boost::mutex::scoped_lock sl(_mutexServices);
      std::map<unsigned int, qi::Object>::iterator it;
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
            unsigned int linkId = it->second.connect(event,
              boost::bind(&forwardEvent, _1, service, event, client));
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
              it->second.disconnect(i->second.localLinkId);
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
          case Message::ServerFunction_MetaObject:
          {
            unsigned int objectId;
            ds >> objectId;
            std::cout << "Metaobject for: " << service << "-" << objectId << std::endl;
            it = _services.find(service);
            if (it == _services.end()) {
              qiLogError("qi.server") << "No object FOund. implementation failure";
              return;
            }
            obj = it->second;
            //if (msg.type() == Message::Type_Call)

            {
              qi::Message retval;
              retval.buildReplyFrom(msg);
              Buffer buf;
              ODataStream ds(buf);
              ds << obj.metaObject();
              retval.setBuffer(buf);
              client->send(retval);
            }
            break;
          }
        }
        return;
      } // msg.service() == Server
      it = _services.find(msg.service());
      obj = it->second;
      if (it == _services.end() || !obj.isValid())
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
         qi::Future<MetaFunctionResult> fut = obj.metaCall(msg.function(), MetaFunctionParameters(msg.buffer()), qi::Object::MetaCallType_Queued);
         fut.addCallbacks(new ServerResult(client, msg));
      }
      break;
    case Message::Type_Event:
      {
        obj.metaEmit(msg.function(), MetaFunctionParameters(msg.buffer()));
      }
      break;
    }

  };

  void Session_Server::onFutureFailed(const std::string &error, void *data)
  {
    qi::ServiceInfo si;
    long id = reinterpret_cast<long>(data);
    RegisterServiceMap::iterator it = _servicesObject.find(id);
    if (it != _servicesObject.end())
      _servicesObject.erase(it);
  }

  void Session_Server::onFutureFinished(const unsigned int &idx,
                                        void               *data)
  {
    long id = reinterpret_cast<long>(data);
    qi::ServiceInfo si;
    RegisterServiceMap::iterator it;

    {
      boost::recursive_mutex::scoped_lock sl(_mutexOthers);
      it = _servicesObject.find(id);
      if (it != _servicesObject.end())
        si = it->second.second;
    }
    si.setServiceId(idx);

    {
      boost::mutex::scoped_lock sl(_mutexServices);
      _services[idx] = it->second.first;
    }
    // ack the Service directory to tell that we are ready
    _sdClient->serviceReady(idx);
    {
      boost::recursive_mutex::scoped_lock sl(_mutexOthers);
      _servicesInfo[si.name()] = si;
      _servicesByName[si.name()] = it->second.first;
      _servicesIndex[idx] = si.name();
      _servicesObject.erase(it);
    }
  }


  bool Session_Server::listen(const std::string &address)
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

  qi::Future<unsigned int> Session_Server::registerService(const std::string &name,
                                                           const qi::Object  &obj)
  {
    if (_server.endpoints().empty()) {
      qiLogError("qimessaging.Server") << "Could not register service: " << name << " because the current server has not endpoint";
      return qi::Future<unsigned int>();
    }
    qi::ServiceInfo si;
    si.setName(name);
    si.setProcessId(qi::os::getpid());
    si.setMachineId(qi::os::getMachineId());


    {
      std::vector<qi::Url> epsUrl = _server.endpoints();
      std::vector<std::string> epsStr;
      for (std::vector<qi::Url>::const_iterator epsUrlIt = epsUrl.begin();
           epsUrlIt != epsUrl.end();
           epsUrlIt++)
      {
        epsStr.push_back((*epsUrlIt).str());
      }
      si.setEndpoints(epsStr);
    }

    long id = ++_servicesObjectIndex;
    {
      boost::recursive_mutex::scoped_lock sl(_mutexOthers);
      _servicesObject[id] = std::make_pair(obj, si);
    }

    qi::Future<unsigned int> future;
    future = _sdClient->registerService(si);
    future.addCallbacks(this, reinterpret_cast<void *>(id));

    return future;
  };

  qi::Future<void> Session_Server::unregisterService(unsigned int idx)
  {
    qi::Future<void> future = _sdClient->unregisterService(idx);


    {
      boost::mutex::scoped_lock sl(_mutexServices);
      _services.erase(idx);
    }
    {
      boost::recursive_mutex::scoped_lock sl(_mutexOthers);
      std::map<unsigned int, std::string>::iterator it;
      it = _servicesIndex.find(idx);
      if (it == _servicesIndex.end()) {
        qiLogError("qimessaging.Server") << "Can't find name associated to id:" << idx;
      }
      else {
        _servicesByName.erase(it->second);
        _servicesInfo.erase(it->second);
      }
      _servicesIndex.erase(idx);
    }
    return future;
  };

  void Session_Server::close() {
    _server.close();
  }

  std::vector<qi::ServiceInfo> Session_Server::registeredServices() {
    std::vector<qi::ServiceInfo> ssi;
    std::map<std::string, qi::ServiceInfo>::iterator it;

    {
      boost::recursive_mutex::scoped_lock sl(_mutexOthers);
      for (it = _servicesInfo.begin(); it != _servicesInfo.end(); ++it) {
        ssi.push_back(it->second);
      }
    }
    return ssi;
  }

  qi::ServiceInfo Session_Server::registeredService(const std::string &service) {
    std::map<std::string, qi::ServiceInfo>::iterator it;
    {
      boost::recursive_mutex::scoped_lock sl(_mutexOthers);
      it = _servicesInfo.find(service);
      if (it != _servicesInfo.end())
        return it->second;
    }
    return qi::ServiceInfo();
  }

  qi::Object Session_Server::registeredServiceObject(const std::string &service) {
    std::map<std::string, qi::Object>::iterator it;
    {
      boost::recursive_mutex::scoped_lock sl(_mutexOthers);
      it = _servicesByName.find(service);
      if (it != _servicesByName.end())
        return it->second;
    }
    return Object();
  }

  qi::Url Session_Server::listenUrl() const {
    return _server.listenUrl();
  }

}
