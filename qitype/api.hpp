#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QIMESSAGING_API_HPP_
#define _QIMESSAGING_API_HPP_

#include <qi/macro.hpp>

#ifdef qitype_EXPORTS
# define QITYPE_API QI_EXPORT_API
#else
# define QITYPE_API QI_IMPORT_API
#endif

#endif  // _QIMESSAGING_API_HPP_
