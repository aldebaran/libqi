#pragma once
/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef AL_MESSAGING_SERIALIZATION_SERIALIZATION_HPP_
#define AL_MESSAGING_SERIALIZATION_SERIALIZATION_HPP_

#include <alcommon-ng/serialization/boost/boost_binary_serializer.hpp>

namespace AL {
  namespace Serialization {
    typedef BoostBinarySerializer Serializer;
  }
}

#endif  // AL_MESSAGING_SERIALIZATION_SERIALIZATION_HPP_
