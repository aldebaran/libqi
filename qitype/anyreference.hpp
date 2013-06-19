#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_ANYREFERENCE_HPP_
#define _QITYPE_ANYREFERENCE_HPP_

//AnyReference/AnyIterator are declared in their own files to avoid
//include dependencies cycles...
#include <qitype/details/anyreference.hpp>
//when using AnyReference you need type. so include it!
#include <qitype/type.hpp>
#include <qitype/details/anyiterator.hpp>

#endif  // _QITYPE_ANYREFERENCE_HPP_
