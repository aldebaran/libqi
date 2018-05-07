#pragma once
#ifndef QI_UUID_HPP
#define QI_UUID_HPP
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>

/// Simple aliases to abstract a bit from implementation.

namespace qi
{
  using Uuid = boost::uuids::uuid;
  using UuidRandomGenerator = boost::uuids::random_generator;
} // qi

#endif // QI_UUID_HPP
