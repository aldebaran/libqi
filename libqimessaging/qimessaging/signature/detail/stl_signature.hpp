#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_SIGNATURE_DETAIL_STL_SIGNATURE_HPP_
#define _QI_SIGNATURE_DETAIL_STL_SIGNATURE_HPP_

# include <qimessaging/signature/detail/type_signature.hpp>
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

#endif  // _QI_SIGNATURE_DETAIL_TYPE_SIGNATURE_HPP_
