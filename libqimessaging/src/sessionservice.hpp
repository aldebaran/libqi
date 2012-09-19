/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef    SESSION_SERVICE_HPP_
# define    SESSION_SERVICE_HPP_

#include <qimessaging/future.hpp>
#include <string>
#include <boost/thread/mutex.hpp>
#include <qimessaging/session.hpp>
#include <qi/atomic.hpp>
#include "src/remoteobject_p.hpp"

namespace qi {

  class GenericObject;
  class ServiceDirectoryClient;
  class Session_Server;
  class ServerClient;

  struct ServiceRequest
  {

    ServiceRequest(const std::string &service = "", const std::string &protocol = "")
      : name(service)
      , serviceId(0)
      , protocol(protocol)
      , connected(false)
      , attempts(0)
      , socket()
      , sclient(0)
    {}

    qi::Promise<qi::GenericObject> promise;
    std::string                   name;
    unsigned int                  serviceId;
    std::string                   protocol;
    bool                          connected; // True if the service server was reached
    unsigned int                  attempts;  // Number of connection attempts pending.
    TransportSocketPtr            socket;
    ServerClient                 *sclient;
  };

  class Session_Service : public FutureInterface<qi::ServiceInfo>,
                          public FutureInterface<qi::MetaObject>
  {
  public:
    Session_Service(ServiceDirectoryClient *sdClient, Session_Server *server)
      : _sdClient(sdClient)
      , _server(server)
    {}
    ~Session_Service();

    void close();

    qi::Future<qi::GenericObject> service(const std::string &service,
                                   Session::ServiceLocality locality,
                                   const std::string &protocol);

  protected:
    //FutureInterface
    virtual void onFutureFailed(const std::string &error, void *data);
    virtual void onFutureFinished(const qi::ServiceInfo &value, void *data);
    virtual void onFutureFinished(const qi::MetaObject &value, void *data);

    //TransportSocket
    void onSocketConnected(qi::TransportSocketPtr socket, void *data);
    void onSocketDisconnected(qi::TransportSocketPtr socket, int error, void *data);


  protected:
    ServiceRequest *serviceRequest(void *data);
    void            removeRequest(void *data);

  protected:
    boost::mutex                    _requestsMutex;
    std::map<long, ServiceRequest*> _requests;
    qi::atomic<long>                _requestsIndex;

    //maintain a cache of remote object
    typedef std::map<std::string, GenericObject> RemoteObjectMap;
    RemoteObjectMap                 _remoteObjects;
    boost::mutex                    _remoteObjectsMutex;

    //maintain a cache of remote connections
    typedef std::map<std::string, qi::TransportSocketPtr> TransportSocketMap;
    TransportSocketMap              _sockets;

  private:
    ServiceDirectoryClient *_sdClient;  //not owned by us
    Session_Server         *_server;    //not owned by us
  };

}
#endif     /* !SESSION_SERVICE_PP_ */
