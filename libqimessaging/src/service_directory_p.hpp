/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef  SERVICE_DIRECTORY_P_HPP_
# define SERVICE_DIRECTORY_P_HPP_

#include <qimessaging/transport_server.hpp>
#include <qimessaging/transport_socket.hpp>
#include <qimessaging/object.hpp>
#include <set>
#include <boost/thread/recursive_mutex.hpp>
#include <qimessaging/functor.hpp>

namespace qi {

  class ServiceDirectoryPrivate : public TransportServerInterface,
                                  public TransportSocketInterface,
                                  public Object
  {
  public:
    ServiceDirectoryPrivate();
    ~ServiceDirectoryPrivate();

    //TransportServer
    virtual void newConnection(TransportServer* server, TransportSocket *socket);

    //TransportSocket
    virtual void onSocketReadyRead(TransportSocket *socket, int id, void *data);
    virtual void onSocketWriteDone(TransportSocket *client, void *data);
    virtual void onSocketConnected(TransportSocket *client, void *data);
    virtual void onSocketDisconnected(TransportSocket *client, void *data);

    std::vector<ServiceInfo> services();
    ServiceInfo              service(const std::string &name);
    unsigned int             registerService(const ServiceInfo &svcinfo);
    void                     unregisterService(const unsigned int &idx);
    TransportSocket         *socket() { return currentSocket; }
    void                     serviceReady(const unsigned int &idx);
  public:
    Session                                               *session;
    qi::TransportServer                                   *ts;
    std::map<unsigned int, ServiceInfo>                    pendingServices;
    std::map<unsigned int, ServiceInfo>                    connectedServices;
    std::map<std::string, unsigned int>                    nameToIdx;
    std::map<TransportSocket*, std::vector<unsigned int> > socketToIdx;
    unsigned int                                           servicesCount;
    TransportSocket                                       *currentSocket;
    std::set<TransportSocket*>                             _clients;
    boost::recursive_mutex                                 _clientsMutex;
  }; // !ServiceDirectoryPrivate

}

#endif /* !SERVICE_DIRECTORY_P_PP_ */
