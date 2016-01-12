#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/


#ifndef _QI_TYPES_HPP_
# define _QI_TYPES_HPP_

/**
 * \includename{qi/types.hpp}
 */

/**
 * \using qi::int8_t
 * \brief Deprecated in 2.5. Use int8_t from <cstdint>.
 */

/**
 * \using qi::int16_t
 * \brief Deprecated in 2.5. Use int16_t from <cstdint>.
 */

/**
 * \using qi::int32_t
 * \brief Deprecated in 2.5. Use int32_t from <cstdint>.
 */

/**
 * \using qi::int64_t
 * \brief Deprecated in 2.5. Use int64_t from <cstdint>.
 */

/**
 * \using qi::uint8_t
 * \brief Deprecated in 2.5. Use uint8_t from <cstdint>.
 */

/**
 * \using qi::uint16_t
 * \brief Deprecated in 2.5. Use uint16_t from <cstdint>.
 */

/**
 * \using qi::uint32_t
 * \brief Deprecated in 2.5. Use uint32_t from <cstdint>.
 */

/**
 * \using qi::uint64_t
 * \brief Deprecated in 2.5. Use uint64_t from <cstdint>.
 */

#  include <cstdint>
namespace qi
{
  using int8_t = int8_t ;
  using int16_t = int16_t;
  using int32_t = int32_t;
  using int64_t = int64_t;

  using uint8_t = uint8_t;
  using uint16_t = uint16_t;
  using uint32_t = uint32_t;
  using uint64_t = uint64_t;
}

#endif  // _QI_TYPES_HPP_
