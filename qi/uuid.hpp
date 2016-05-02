#pragma once
/*
**  Copyright (C) 2016 Aldebaran Robotics
**  See COPYING for the license
*/

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>

/// Simple aliases to make the code a little easier to read.

namespace qi
{
  using Uuid = boost::uuids::uuid;
  using UuidRandomGenerator = boost::uuids::random_generator;
} // qi
