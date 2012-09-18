/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef  SERVICE_DIRECTORY_CLIENT_HPP_
# define SERVICE_DIRECTORY_CLIENT_HPP_

#include <vector>
#include <qimessaging/service_info.hpp>
#include <qimessaging/session.hpp>
#include "remoteobject_p.hpp"

namespace qi {

  class TransportSocket;
  class ServiceDirectoryClient {
  public:
    ServiceDirectoryClient();
    ~ServiceDirectoryClient();

    qi::FutureSync<bool> connect(const qi::Url &serviceDirectoryURL);
    bool isConnected() const;
    qi::Url url() const;
    qi::FutureSync<void> disconnect();

    void addCallbacks(SessionInterface *delegate, void *data);
    void removeCallbacks(SessionInterface *delegate);

    qi::Future< std::vector<ServiceInfo> > services();
    qi::Future< ServiceInfo >              service(const std::string &name);
    qi::Future< unsigned int >             registerService(const ServiceInfo &svcinfo);
    qi::Future< void >                     unregisterService(const unsigned int &idx);
    qi::Future< void >                     serviceReady(const unsigned int &idx);

  protected:
    void onSocketDisconnected(TransportSocketPtr client, void *data);

  private:
    std::vector< std::pair<qi::SessionInterface *, void *> > _callbacks;
    boost::mutex                                             _callbacksMutex;

    //stored only for callbacks
    qi::Session         *_session;
  public:
    TransportSocketPtr   _socket;
  private:
    qi::RemoteObject     _object;
  };
}

#endif
