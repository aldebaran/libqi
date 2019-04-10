#pragma once
#ifndef QI_UUID_HPP
#define QI_UUID_HPP
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/random/random_device.hpp>

/// Simple aliases to abstract a bit from implementation.

namespace qi
{
  using Uuid = boost::uuids::uuid;
  using UuidRandomGenerator =
    boost::uuids::basic_random_generator<boost::random::random_device>;
  // We don't use the default random uuid generator because in Boost < 1.67 it uses
  // a Mersenne Twister engine which is randomness is not cryptographically secure.
  // Use Boost.Random 's random device engine as it provides that guarantee.
  // TODO: Boost >= v1.67 : use either boost::uuids::random_generator
  // or explicitely boost::uuids::random_generator_pure for optimal case.

} // qi

#endif // QI_UUID_HPP
