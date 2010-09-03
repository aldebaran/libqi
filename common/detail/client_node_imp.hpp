#pragma once
/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/
#ifndef COMMON_CLIENT_NODE_IMP_HPP_
#define COMMON_CLIENT_NODE_IMP_HPP_

#include <string>
#include <alcommon-ng/messaging/client.hpp>
#include <alcommon-ng/common/detail/mutexednamelookup.hpp>
#include <alcommon-ng/common/detail/namelookup.hpp>
#include <alcommon-ng/messaging/call_definition.hpp>
#include <alcommon-ng/messaging/result_definition.hpp>

namespace AL {
  namespace Common {

    class ClientNodeImp {
    public:
      ClientNodeImp();

      virtual ~ClientNodeImp();

      ClientNodeImp(const std::string& clientName,
        const std::string& masterAddress);

      void call(const AL::Messaging::CallDefinition& callDef, AL::Messaging::ResultDefinition &result);

      bool initOK;

    private:
      std::string fClientName;
      std::string fMasterAddress;

      MutexedNameLookup<std::string> fServiceCache;

      // map from address to Client
      NameLookup<boost::shared_ptr<AL::Messaging::Client> > fServerClients;

      void xInit();
      bool xCreateServerClient(const std::string& address);
      const std::string xLocateService(const std::string& methodHash);

    };
  }
}

#endif  // COMMON_CLIENT_NODE_IMP_HPP_
