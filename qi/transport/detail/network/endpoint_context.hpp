#pragma once
/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/
#ifndef   __QI_MESSAGING_DETAIL_ENDPOINT_CONTEXT_HPP__
#define   __QI_MESSAGING_DETAIL_ENDPOINT_CONTEXT_HPP__

#include <string>

namespace qi {
  namespace detail {

    struct EndpointContext {
      std::string endpointID;
      std::string machineID;
      std::string hostName;
      int processID;
      int platformID;
      std::string publicIP;
      // not discoverable at construction
      std::string name;
      std::string contextID;
      int port;
      EndpointContext();
    };

//advertiseService(sig, myID)
//int port = registerServer(name, myID, myContextID, myMachineID, myPlatformID, myPublicIP)
//bool registerClient(mame, myID, myContextID, myMachineID, myPlatformID)

//call(sig, stuff)
//endpoint = locate(sig, clientContextID)
//  clientContext = clientsContexts.find(clientContextID);
//  serverContextID = serviceCache.find(sig);
//  serverContext = serviceContexts.find(serverContextID);
//  endpoint = negotiateProtocol(clientContext, serverContext)


  }
}
#endif // __QI_MESSAGING_DETAIL_ENDPOINT_CONTEXT_HPP__

