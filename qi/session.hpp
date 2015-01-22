#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QI_MESSAGING_SESSION_HPP_
#define _QI_MESSAGING_SESSION_HPP_

#include <qi/api.hpp>
#include <qi/messaging/serviceinfo.hpp>
#include <qi/messaging/authproviderfactory.hpp>
#include <qi/messaging/clientauthenticatorfactory.hpp>
#include <qi/future.hpp>
#include <qi/anyobject.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>
#include <string>

#ifdef _MSC_VER
#  pragma warning( push )
#  pragma warning( disable: 4251 )
#endif

namespace qi {

  class SessionPrivate;
  class AuthProvider;
  typedef boost::shared_ptr<AuthProvider> AuthProviderPtr;
  typedef std::map<std::string, AnyValue> CapabilityMap;

  /** A Session allows you to interconnect services on the same machine or over
   * the network.
   *
   * \includename{qi/session.hpp}
   */
  class QI_API Session : boost::noncopyable, public ::boost::enable_shared_from_this<Session> {
  public:
    Session(bool enforceAuthentication = false);
    virtual ~Session();

    enum ServiceLocality {
      ServiceLocality_All    = 0,
      ServiceLocality_Local  = 1,
      ServiceLocality_Remote = 2
    };

    //Client
    qi::FutureSync<void> connect(const char* serviceDirectoryURL);
    qi::FutureSync<void> connect(const std::string &serviceDirectoryURL);
    qi::FutureSync<void> connect(const qi::Url &serviceDirectoryURL);
    bool isConnected() const;
    qi::Url url() const;

    qi::FutureSync< std::vector<ServiceInfo> > services(ServiceLocality locality = ServiceLocality_All);

    qi::FutureSync< qi::AnyObject > service(const std::string &aservice)
    {
      return service(aservice, "");
    }

    qi::FutureSync< qi::AnyObject > service(const std::string &service,
                                            const std::string &protocol);

    //Server
    qi::FutureSync<void> listen(const qi::Url &address);
    std::vector<qi::Url> endpoints() const;
    bool    setIdentity(const std::string& key, const std::string& crt);

    //close both client and server side
    qi::FutureSync<void>    close();

    //this create a listen and create a service directory
    qi::FutureSync<void> listenStandalone(const qi::Url &address);

    qi::FutureSync<unsigned int> registerService(const std::string &name, AnyObject object);
    qi::FutureSync<void>         unregisterService(unsigned int serviceId);


    void setAuthProviderFactory(AuthProviderFactoryPtr);
    void setClientAuthenticatorFactory(ClientAuthenticatorFactoryPtr);

    /** Load a module and register the specified object on the session
     *
     * Tries to call the factory with (this, args...) if possible, otherwise it
     * calls it with (args...) only.
     */
    void loadService(const std::string& moduleName, const std::string &renameModule = "", const AnyReferenceVector& args = AnyReferenceVector());

#define pushi(z, n, _) params.push_back(p ## n);
#define genCall(n, ATYPEDECL, ATYPES, ADECL, AUSE, comma)             \
  void loadService(                                                   \
      const std::string& moduleName, const std::string& renameModule, \
      qi::AutoAnyReference pp0 comma                                  \
      QI_GEN_ARGSDECLSAMETYPE(n, qi::AutoAnyReference))               \
  {                                                                   \
    std::vector<qi::AnyReference> params;                             \
    params.reserve(n+1);                                              \
    params.push_back(pp0);                                            \
    BOOST_PP_REPEAT(n, pushi, _)                                      \
    loadService(moduleName, renameModule, params);                    \
  }
QI_GEN(genCall)
#undef genCall
#undef pushi

    /** Waits for a service to become available. The future is set immediately
     * if the service is already available.
     */
    qi::FutureSync<void> waitForService(const std::string& service);

  public:
    qi::Signal<unsigned int, std::string> serviceRegistered;
    qi::Signal<unsigned int, std::string> serviceUnregistered;
    // C4251
    qi::Signal<>                          connected;
    // C4251
    qi::Signal<std::string>               disconnected;

  protected:
    friend class SessionPrivate;
    boost::shared_ptr<SessionPrivate>    _p;
  };

  typedef boost::shared_ptr<Session> SessionPtr;

  inline SessionPtr makeSession() { return boost::make_shared<qi::Session>(); }
}

QI_TYPE_ENUM_REGISTER(qi::Session::ServiceLocality);

#ifdef _MSC_VER
#  pragma warning( pop )
#endif

#endif  // _QIMESSAGING_SESSION_HPP_
