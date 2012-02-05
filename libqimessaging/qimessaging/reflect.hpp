#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010, 2011 Aldebaran Robotics
*/


#ifndef _QIMESSAGING_REFLECT_HPP_
#define _QIMESSAGING_REFLECT_HPP_

#include <qimessaging/api.hpp>
#include <qimessaging/signature.hpp>
#include <qimessaging/message.hpp>

#define QI_REFLECT(TYPE, MEMBERS)                                                   \
  QI_SIGNATURE_REFLECT(TYPE, MEMBERS);                                              \
  QI_SERIALIZATION_REFLECT(TYPE, MEMBERS);


#endif  // _QIMESSAGING_REFLECT_HPP_
