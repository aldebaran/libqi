/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef   __QI_SIGNATURE_DETAIL_PROTOBUF_SIGNATURE_HPP__
#define   __QI_SIGNATURE_DETAIL_PROTOBUF_SIGNATURE_HPP__


# include <boost/utility.hpp>
# include <boost/type_traits/is_base_of.hpp>
# include <protobuf/message.h>

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

#endif // __QI_SIGNATURE_DETAIL_TYPESIGNATURE_HPP__
