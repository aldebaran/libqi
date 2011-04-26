#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_SIGNATURE_DETAIL_PROTOBUF_SIGNATURE_HPP_
#define _QI_SIGNATURE_DETAIL_PROTOBUF_SIGNATURE_HPP_


# include <boost/utility.hpp>
# include <boost/type_traits/is_base_of.hpp>
# include <google/protobuf/message.h>

namespace qi {
  namespace detail {

    template <typename T>
    struct signature<T, typename boost::enable_if< typename boost::is_base_of<google::protobuf::Message , T>::type >::type > {
      static std::string &value(std::string &val) {
        val += "@";
        //TODO: quite unefficient no?
        static T p;
        val += p.GetTypeName();
        val += "@";
        return val;
      }
    };

  }
}

#endif  // _QI_SIGNATURE_DETAIL_PROTOBUF_SIGNATURE_HPP_
