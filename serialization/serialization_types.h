#pragma once
/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/
#ifndef SERIALIZATION_SERIALIZATION_TYPES_H_
#define SERIALIZATION_SERIALIZATION_TYPES_H_

namespace AL {
  namespace Serialization {
    enum SERIALIZATION_TYPE {
      BOOST_BINARY,
      BOOST_TEXT,
      BOOST_XML
    };
  }
}

#endif  // SERIALIZATION_SERIALIZATION_TYPES_H_
