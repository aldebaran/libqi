#pragma once
/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef SERIALIZATION_SERIALIZATION_H_
#define SERIALIZATION_SERIALIZATION_H_

#include "boost_serialization.h"

namespace AL {
  namespace Serialization {
    typedef BoostSerializer Serializer;
  }
}

#endif  // SERIALIZATION_SERIALIZATION_H_
