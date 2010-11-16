#pragma once
/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/
#ifndef QI_NODE_DETAIL_GET_PID_HPP
#define QI_NODE_DETAIL_GET_PID_HPP

#ifdef _WIN32

#include <windows.h>
#include <winsock2.h>
#include <iphlpapi.h>
#pragma comment(lib, "IPHLPAPI.lib")

int getProcessID() {
  return(int)  GetCurrentProcessId();
}

std::string getHostName() {
  std::string hostname = "";
  char szHostName[128] = "";
  // Requires ws2_32.lib
  WSADATA WSAData;
  // Initialize winsock dll
  if(::WSAStartup(MAKEWORD(1, 0), &WSAData) == FALSE) {
    // error
  } else {
    if(::gethostname(szHostName, sizeof(szHostName) - 1) == 0) {
      hostname = std::string(szHostName);
    }
  }
  WSACleanup();
  return hostname;
}

std::string getFirstMacAddress() {
  std::string macAddress;

  // Reduced version of http://msdn.microsoft.com/en-us/library/aa365915(VS.85).aspx
  PIP_ADAPTER_ADDRESSES pAddresses = NULL;
  DWORD dwRetVal = 0;
  ULONG outBufLen = 15000;
  ULONG Iterations = 0;

  do {
    pAddresses = (IP_ADAPTER_ADDRESSES *) malloc(outBufLen);
    if (pAddresses == NULL) {
      return macAddress;
    }
    dwRetVal =
      GetAdaptersAddresses(AF_INET, GAA_FLAG_INCLUDE_PREFIX, NULL, pAddresses, &outBufLen);
    if (dwRetVal == ERROR_BUFFER_OVERFLOW) {
      free(pAddresses);
      pAddresses = NULL;
    } else {
      break;
    }
    Iterations++;
  } while ((dwRetVal == ERROR_BUFFER_OVERFLOW) && (Iterations < 3));

  if (dwRetVal == NO_ERROR) {
    // we are just interested in the first mac address
    if (pAddresses) {
      macAddress = pAddresses->AdapterName;
    }
  }
  if (pAddresses) {
    free(pAddresses);
  }
  return macAddress;
}
#else  // end WIN32

# include <sys/types.h>
# include <unistd.h>

int getProcessID() {
  return getpid();
}

std::string getHostName() {
  char szHostName[128] = "";
  if (gethostname(szHostName, sizeof(szHostName) -1) == 0) {
    return std::string(szHostName);
  }
  return "";
}

std::string getFirstMacAddress() {
  std::string macAddress;
  // TODO implement
  return macAddress;
}
#endif

#endif  // QI_NODE_DETAIL_GET_PID_HPP

