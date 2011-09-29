#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010, 2011 Aldebaran Robotics
*/


#ifndef _QIMESSAGING_SERIALIZATION_SERIALIZE_STL_HXX_
#define _QIMESSAGING_SERIALIZATION_SERIALIZE_STL_HXX_

namespace qi {
  namespace serialization {

    QI_SIMPLE_SERIALIZER(String, std::string);
    QI_LIST_SERIALIZER(std::vector);
    QI_LIST_SERIALIZER(std::list);
    QI_MAP_SERIALIZER(std::map);

  }
}

#endif  // _QIMESSAGING_SERIALIZATION_SERIALIZE_STL_HXX_
