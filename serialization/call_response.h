#pragma once
/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef SERIALIZATION_CALLRESPONSE_H_
#define SERIALIZATION_CALLRESPONSE_H_

template<typename T>
struct CallResponse {
  T response;
};

#endif  // SERIALIZATION_CALLRESPONSE_H_