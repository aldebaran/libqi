#pragma once
/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef SERIALIZATION_CALLREQUEST_H_
#define SERIALIZATION_CALLREQUEST_H_

template<typename T>
struct CallRequest {
  std::string module;
  std::string method;
  T args;
};

#endif  // SERIALIZATION_CALLREQUEST_H_