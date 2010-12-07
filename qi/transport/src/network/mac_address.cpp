/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#include <qi/transport/detail/network/mac_address.hpp>

#ifdef _WIN32
#include <windows.h>
#include <winsock2.h>
#include <iphlpapi.h>
#pragma comment(lib, "IPHLPAPI.lib")

namespace qi {
  namespace detail {

    std::string getFirstMacAddress() {
      // BEWARE: Takes about 1.6 ms
      // Get the buffer length required for IP_ADAPTER_INFO.
      ULONG BufferLength = 0;
      BYTE* pBuffer = 0;
      if( ERROR_BUFFER_OVERFLOW == GetAdaptersInfo( 0, &BufferLength ))
      {
        // Now the BufferLength contain the required buffer length.
        // Allocate necessary buffer.
        pBuffer = new BYTE[ BufferLength ];
      } else {
        return "";
      }
      // Get the Adapter Information.
      PIP_ADAPTER_INFO pAdapterInfo =
        reinterpret_cast<PIP_ADAPTER_INFO>(pBuffer);
      GetAdaptersInfo( pAdapterInfo, &BufferLength );

      char buf[25];
      // Iterate the network adapters and print their MAC address.
      while( pAdapterInfo )
      {
        if (pAdapterInfo->AddressLength == 6) {
          //IP4
        sprintf(buf, "%02X:%02X:%02X:%02X:%02X:%02X",
          pAdapterInfo->Address[0],
          pAdapterInfo->Address[1],
          pAdapterInfo->Address[2],
          pAdapterInfo->Address[3],
          pAdapterInfo->Address[4],
          pAdapterInfo->Address[5]);
        } else if (pAdapterInfo->AddressLength == 8) {
          // IP6
          sprintf(buf, "%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X",
          pAdapterInfo->Address[0],
          pAdapterInfo->Address[1],
          pAdapterInfo->Address[2],
          pAdapterInfo->Address[3],
          pAdapterInfo->Address[4],
          pAdapterInfo->Address[5],
          pAdapterInfo->Address[6],
          pAdapterInfo->Address[7]);
        }
        // we are just interested in the first one
        break;
        // Get next adapter info.
        // pAdapterInfo = pAdapterInfo->Next;
      }

      // deallocate the buffer.
      delete[] pBuffer;
      return buf;
    }
  }
}

#else  // end WIN32

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <netdb.h>
#include <stdio.h>
#include <cstring>

namespace qi {
  namespace detail {
    std::string getFirstMacAddress() {
      std::string macAddress = getMacAddress("eth0");
      if (macAddress.empty()) {
        macAddress = getMacAddress("wlan0");
      }
      return macAddress;
    }

    std::string getMacAddress( std::string pIfInterface )
    {
      char buf[25];          // alloc for formatted string

      struct ifreq s;
      int fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
      if (fd < 0) {
        return buf;
      }
      strcpy(s.ifr_name, "eth0");
      if (0 == ioctl(fd, SIOCGIFHWADDR, &s)) {
        sprintf(buf, "%02X:%02X:%02X:%02X:%02X:%02X",
                (unsigned char) s.ifr_addr.sa_data[0],
                (unsigned char) s.ifr_addr.sa_data[1],
                (unsigned char) s.ifr_addr.sa_data[2],
                (unsigned char) s.ifr_addr.sa_data[3],
                (unsigned char) s.ifr_addr.sa_data[4],
                (unsigned char) s.ifr_addr.sa_data[5]);
      }
      close(fd);
      return buf;
    }
  }
}

#endif  // !_WIN32
