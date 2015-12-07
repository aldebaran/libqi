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
 * \typedef qi::int8_t
 * \brief Deprecated in 2.5. Use int8_t from <cstdint>.
 */

/**
 * \typedef qi::int16_t
 * \brief Deprecated in 2.5. Use int16_t from <cstdint>.
 */

/**
 * \typedef qi::int32_t
 * \brief Deprecated in 2.5. Use int32_t from <cstdint>.
 */

/**
 * \typedef qi::int64_t
 * \brief Deprecated in 2.5. Use int64_t from <cstdint>.
 */

/**
 * \typedef qi::uint8_t
 * \brief Deprecated in 2.5. Use uint8_t from <cstdint>.
 */

/**
 * \typedef qi::uint16_t
 * \brief Deprecated in 2.5. Use uint16_t from <cstdint>.
 */

/**
 * \typedef qi::uint32_t
 * \brief Deprecated in 2.5. Use uint32_t from <cstdint>.
 */

/**
 * \typedef qi::uint64_t
 * \brief Deprecated in 2.5. Use uint64_t from <cstdint>.
 */

#  include <cstdint>
namespace qi
{
  typedef int8_t   int8_t;
  typedef int16_t  int16_t;
  typedef int32_t  int32_t;
  typedef int64_t  int64_t;

  typedef uint8_t  uint8_t;
  typedef uint16_t uint16_t;
  typedef uint32_t uint32_t;
  typedef uint64_t uint64_t;
}

#endif  // _QI_TYPES_HPP_
