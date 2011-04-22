#pragma once
/*
 *  Copyright (C) 2011 Aldebaran Robotics
 */

#ifndef _QIMESSAGING_API_H_
#define _QIMESSAGING_API_H_

#include <qi/api.hpp>

#ifdef qimessaging_EXPORTS
# define QIMESSAGING_API QI_EXPORT_API
#else
# define QIMESSAGING_API QI_IMPORT_API
#endif

#endif  // _LIB_ALCORE_ALCORE_ALAPI_H_

