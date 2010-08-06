#pragma once
/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef SERIALIZATION_CALLRESPONSE_SERIALIZATION_H_
#define SERIALIZATION_CALLRESPONSE_SERIALIZATION_H_

#include "call_response.h"
#include <boost/serialization/nvp.hpp>

namespace boost {
  namespace serialization {

    /// <summary> Serializes a CallResponse </summary>
    /// <param name="ar">      [in,out] the archive. </param>
    /// <param name="item">    [in,out] the CallResponse item. </param>
    /// <param name="version"> The version. </param>
    template<class Archive>
    void serialize(Archive & ar, CallResponse& item,
      const unsigned int version) {
          ar & make_nvp("response", item.response);
    }
  }
}

#endif  // SERIALIZATION_CALLRESPONSE_SERIALIZATION_H_
