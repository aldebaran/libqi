/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef    SESSION_SERVICES_HPP_
# define    SESSION_SERVICES_HPP_

#include <qimessaging/future.hpp>
#include <vector>
#include <boost/thread/mutex.hpp>
#include <qimessaging/session.hpp>
#include <qi/atomic.hpp>

namespace qi {

  struct ServicesRequest {
  public:
    ServicesRequest()
    {}

    ServicesRequest(qi::Promise< std::vector<qi::ServiceInfo> > promise, Session::ServiceLocality locality)
      : promise(promise)
      , locality(locality)
    {}

    qi::Promise< std::vector<qi::ServiceInfo> > promise;
    Session::ServiceLocality                    locality;
  };

  class ServiceDirectoryClient;
  class Session_Server;
  class Session_Services : public qi::FutureInterface< std::vector<qi::ServiceInfo> > {
  public:
    Session_Services(ServiceDirectoryClient *sdClient, Session_Server *server)
      : _sdClient(sdClient)
      , _server(server)
    {}

    qi::Future< std::vector<qi::ServiceInfo> > services(Session::ServiceLocality locality);

  protected:
    //FutureInterface
    virtual void onFutureFailed(const std::string &error, void *data);
    virtual void onFutureFinished(const std::vector<qi::ServiceInfo> &value, void *data);

  protected:
    ServicesRequest *request(void *data);
    void             removeRequest(void *data);

  protected:
    std::map<long, ServicesRequest*>  _request;
    boost::mutex                      _requestMutex;
    qi::atomic<long>                  _requestIndex;

    ServiceDirectoryClient *_sdClient;
    Session_Server         *_server;
  };

}

#endif     /* !SESSION_SERVICES_PP_ */
