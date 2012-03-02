/*
 *  Copyright (C) 2011 Aldebaran Robotics
 */

#pragma once
#ifndef _QIMESSAGING_QT_API_HPP_
#define _QIMESSAGING_QT_API_HPP_

#include <qi/macro.hpp>

#ifdef qimessaging_qt_EXPORTS
# define QIMESSAGING_QT_API QI_EXPORT_API
#else
# define QIMESSAGING_QT_API QI_IMPORT_API
#endif

#endif  // _QIMESSAGING_QT_API_HPP_

