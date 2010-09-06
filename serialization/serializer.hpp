#pragma once
/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef AL_SERIALIZATION_SERIALIZER_HPP_
#define AL_SERIALIZATION_SERIALIZER_HPP_

#include <alcommon-ng/serialization/boost/boost_binary_serializer.hpp>

namespace AL {
  namespace Serialization {
    typedef BoostBinarySerializer Serializer;
  }
}

#endif  // AL_SERIALIZATION_SERIALIZER_HPP_
