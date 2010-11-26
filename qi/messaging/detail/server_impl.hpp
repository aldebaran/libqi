#pragma once
/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/
#ifndef   __QI_MESSAGING_DETAIL_SERVER_IMPL_HPP__
#define   __QI_MESSAGING_DETAIL_SERVER_IMPL_HPP__

#include <string>
#include <qi/transport/message_handler.hpp>
#include <qi/messaging/serviceinfo.hpp>
#include <qi/messaging/detail/mutexednamelookup.hpp>
#include <qi/messaging/context.hpp>
#include <qi/transport/client.hpp>
#include <qi/transport/server.hpp>
#include <qi/transport/detail/network/endpoint_context.hpp>

namespace qi {
  namespace detail {

    class ServerImpl : public qi::transport::MessageHandler {
    public:
      ServerImpl();
      virtual ~ServerImpl();

      ServerImpl(const std::string nodeName,
        const std::string masterAddress);

      const std::string& getName() const;

      void addService(const std::string& methodSignature, qi::Functor* functor);

      bool isInitialized() const;

      const qi::detail::EndpointContext& getEndpointContext() const;

      // MessageHandler Implementation -----------------
      void messageHandler(std::string& defData, std::string& resultData);
      // -----------------------------------------------

    private:
      /// <summary> Becomes true when the server can be used </summary>
      bool _isInitialized;

      /// <summary> true if this server belongs to the master </summary>
      bool _isMasterServer;

      /// <summary> The friendly name of this server </summary>
      std::string _name;

      qi::Context _qiContext;
      qi::detail::EndpointContext _endpointContext;

      /// <summary> The underlying transport server </summary>
      qi::transport::Server _transportServer;

      /// <summary> The transport client used to talk with the master </summary>
      qi::transport:: Client _transportClient;

      // should be map from hash to functor,
      // but we also need to be able to push these hashes to master
      // and ...
      // if would be good if we were capable of describing a mehtod
      MutexedNameLookup<ServiceInfo> _localServices;

      const ServiceInfo& xGetService(const std::string& methodHash);
      void xRegisterServiceWithMaster(const std::string& methodHash);
      void xRegisterSelfWithMaster();
      void xUnregisterSelfWithMaster();
    };
  }
}

#endif // __QI_MESSAGING_DETAIL_SERVER_IMPL_HPP__

