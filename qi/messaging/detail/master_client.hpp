#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_MESSAGING_DETAIL_MASTER_CLIENT_HPP_
#define _QI_MESSAGING_DETAIL_MASTER_CLIENT_HPP_

#include <qi/messaging/detail/impl_base.hpp>
#include <qi/transport/transport_generic_client.hpp>

namespace qi {
  namespace detail {
    class MasterClient: public ImplBase {
    public:
      MasterClient();
      virtual ~MasterClient();

      MasterClient(const std::string name,
                   const std::string& masterAddress);

      void init();

      const std::string& getMasterAddress() const;

      int getNewPort(const std::string& machineID);

      void registerMachine(const qi::detail::MachineContext& m);

      void registerEndpoint(const qi::detail::EndpointContext& e);

      void unregisterEndpoint(const qi::detail::EndpointContext& e);

      std::string locateService(const std::string& methodSignature,
                                const qi::detail::EndpointContext& e);

      void registerService(const std::string& methodSignature,
                           const qi::detail::EndpointContext& e);

      std::string locateTopic(const std::string& methodSignature,
                              const qi::detail::EndpointContext& e);

      bool topicExists(const std::string& topicSignature);

      void registerTopic(const std::string& topicSignature,
                         const qi::detail::EndpointContext& e);

      std::string _masterAddress;

      /// <summary> The transport client used to talk with the master </summary>
      qi::transport::TransportClient _transportClient;
    };
  }
}

#endif  // _QI_MESSAGING_DETAIL_MASTER_CLIENT_HPP_

