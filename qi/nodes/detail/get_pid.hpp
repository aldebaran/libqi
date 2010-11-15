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
# include <windows.h>
#else
# include <sys/types.h>
# include <unistd.h>
#endif

int getProcessID() {
  int pid;

  #ifdef _WIN32
    pid = (int) WINAPI GetCurrentProcessId();
  #else
    pid = getpid();
  #endif

  return pid;
}

//std::string getHostName() {
//  // Add ws2_32.lib to your linker options
//
//  WSADATA WSAData;
//
//  // Initialize winsock dll
//  if(::WSAStartup(MAKEWORD(1, 0), &WSAData) == FALSE)
//    // Error handling
//
//  // Get local host name
//  char szHostName[128] = "";
//
//  if(::gethostname(szHostName, sizeof(szHostName) - 1))
//    // Error -> call 'WSAGetLastError()'
//
//  // Cleanup
//  WSACleanup();
//}

#endif  // QI_NODE_DETAIL_GET_PID_HPP

