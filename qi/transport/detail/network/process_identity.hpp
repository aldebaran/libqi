#pragma once
/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/
#ifndef   __QI_MESSAGING_DETAIL_PROCESS_IDENTITY_HPP__
#define   __QI_MESSAGING_DETAIL_PROCESS_IDENTITY_HPP__

#include <string>
#include <vector>
//#include <uuid.h>

namespace qi {
  namespace detail {

    struct ProcessIdentity {
      int processID;
      std::string hostName;
      std::string macAddress;
      std::string id;
      std::string ipAddress;
      ProcessIdentity();
    };

//    enum Transports {
//      InProc = 0,
//      IPC    = 2,
//      TCP    = 4
//    };
//
//    struct EndpointInfo {
//      uuid_t endpointIdentity;
//      int port;
//      std::vector<Transports> supportedTransports; // yurch
//    };
//
//    struct MachineInfo {
//      std::string hostName;
//      std::string macAddress;
//      int platform;
//      std::vector<std::string> publicIPAddresses;
//      MachineIdentity();
//    };
//
//    struct ContextInfo {
//      uuid_t contextIdentity;
//      int pid;
//    };

    std::string getUUID();

    int getProcessID();

    std::string getHostName();

    // disambiguates hosts
    std::string getFirstMacAddress();
  }
}
#endif // __QI_MESSAGING_DETAIL_PROCESS_IDENTITY_HPP__

