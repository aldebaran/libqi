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

  class Object;
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
      , socket(0)
      , sclient(0)
    {}

    qi::Promise<qi::Object> promise;
    std::string             name;
    unsigned int            serviceId;
    std::string             protocol;
    bool                    connected; // True if the service server was reached
    unsigned int            attempts;  // Number of connection attempts pending.
    TransportSocket*        socket;
    ServerClient           *sclient;
  };

  class Session_Service : public FutureInterface<qi::ServiceInfo>,
                          public FutureInterface<qi::MetaObject>,
                          public TransportSocketInterface
  {
  public:
    Session_Service(ServiceDirectoryClient *sdClient, Session_Server *server)
      : _sdClient(sdClient)
      , _server(server)
    {}
    ~Session_Service();

    void close();

    qi::Future<qi::Object> service(const std::string &service,
                                   Session::ServiceLocality locality,
                                   const std::string &protocol);

  protected:
    //FutureInterface
    virtual void onFutureFailed(const std::string &error, void *data);
    virtual void onFutureFinished(const qi::ServiceInfo &value, void *data);
    virtual void onFutureFinished(const qi::MetaObject &value, void *data);

    //TransportSocket
    void onSocketConnected(TransportSocket *client, void *data);
    void onSocketConnectionError(TransportSocket *QI_UNUSED(client), void *data);
    void onSocketDisconnected(TransportSocket *QI_UNUSED(client), void *data);


  protected:
    ServiceRequest *serviceRequest(void *data);
    void            removeRequest(void *data);

  protected:
    boost::mutex                    _requestsMutex;
    std::map<long, ServiceRequest*> _requests;
    qi::atomic<long>                _requestsIndex;

    //maintain a cache of remote object
    typedef std::map<std::string, qi::RemoteObject> RemoteObjectMap;
    RemoteObjectMap                 _remoteObjects;
    boost::mutex                    _remoteObjectsMutex;

  private:
    ServiceDirectoryClient *_sdClient;
    Session_Server         *_server;
  };

}
#endif     /* !SESSION_SERVICE_PP_ */
