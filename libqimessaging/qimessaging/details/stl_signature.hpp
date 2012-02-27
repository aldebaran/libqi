#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QIMESSAGING_DETAILS_STL_SIGNATURE_HPP_
#define _QIMESSAGING_DETAILS_STL_SIGNATURE_HPP_

# include <qimessaging/details/type_signature.hpp>
# include <string>
# include <list>
# include <vector>
# include <map>

namespace qi {
  namespace detail {

    _QI_SIMPLE_SIGNATURE(std::string, "s");
    _QI_LIST_SIGNATURE(std::list);
    _QI_LIST_SIGNATURE(std::vector);
    _QI_MAP_SIGNATURE(std::map);

  }
}

#endif  // _QIMESSAGING_DETAILS_STL_SIGNATURE_HPP_
