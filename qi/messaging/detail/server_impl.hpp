#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_MESSAGING_DETAIL_SERVER_IMPL_HPP_
#define _QI_MESSAGING_DETAIL_SERVER_IMPL_HPP_

#include <qi/messaging/serviceinfo.hpp>
#include <qi/messaging/detail/mutexednamelookup.hpp>
#include <qi/messaging/detail/master_client.hpp>
#include <qi/transport/message_handler.hpp>
#include <qi/transport/server.hpp>

namespace qi {
  namespace detail {
    class PublisherImpl;

    class ServerImpl : public qi::detail::MasterClient, public qi::transport::MessageHandler {
    public:
      ServerImpl();
      virtual ~ServerImpl();

      ServerImpl(const std::string name,
        const std::string masterAddress);

      const std::string& getName() const;

      void addService(const std::string& methodSignature, qi::Functor* functor);

      boost::shared_ptr<qi::detail::PublisherImpl> advertiseTopic(
        const std::string& topicName,
        const std::string& typeSignature);

      // MessageHandler Implementation -----------------
      void messageHandler(std::string& defData, std::string& resultData);
      // -----------------------------------------------

    protected:
      /// <summary> true if this server belongs to the master </summary>
      bool _isMasterServer;

      /// <summary> The underlying transport server </summary>
      qi::transport::Server _transportServer;

      MutexedNameLookup<ServiceInfo> _localServices;

      const ServiceInfo& xGetService(const std::string& methodHash);
      void xRegisterServiceWithMaster(const std::string& methodHash);
      bool xTopicExists(const std::string& topicName);
    };
  }
}

#endif  // _QI_MESSAGING_DETAIL_SERVER_IMPL_HPP_

