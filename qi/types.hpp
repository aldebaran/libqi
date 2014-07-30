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
 * \brief Cross-platform signed integer of length 8 bits (1 byte).
 */

/**
 * \typedef qi::int16_t
 * \brief Cross-platform signed integer of length 16 bits (2 bytes).
 */

/**
 * \typedef qi::int32_t
 * \brief Cross-platform signed integer of length 32 bits (4 bytes).
 */

/**
 * \typedef qi::int64_t
 * \brief Cross-platform signed integer of length 64 bits (8 bytes).
 */

/**
 * \typedef qi::uint8_t
 * \brief Cross-platform unsigned integer of length 8 bits (1 byte).
 */

/**
 * \typedef qi::uint16_t
 * \brief Cross-platform unsigned integer of length 16 bits (2 bytes).
 */

/**
 * \typedef qi::uint32_t
 * \brief Cross-platform unsigned integer of length 32 bits (4 bytes).
 */

/**
 * \typedef qi::uint64_t
 * \brief Cross-platform unsigned integer of length 64 bits (8 bytes).
 */

// visual studio 2008 and lower version
# if defined(_MSC_VER) && (_MSC_VER <= 1500)
namespace qi
{
  typedef signed __int8     int8_t;
  typedef signed __int16    int16_t;
  typedef signed __int32    int32_t;
  typedef signed __int64    int64_t;

  typedef unsigned __int8   uint8_t;
  typedef unsigned __int16  uint16_t;
  typedef unsigned __int32  uint32_t;
  typedef unsigned __int64  uint64_t;
}
# else
#  include <stdint.h>
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
# endif

#endif  // _QI_TYPES_HPP_
