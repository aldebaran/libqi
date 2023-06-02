#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_DETAIL_FUNCTIONSIGNATURE_HXX_
#define _QITYPE_DETAIL_FUNCTIONSIGNATURE_HXX_

#include <boost/callable_traits.hpp>
#include <qi/signature.hpp>
#include <qi/type/typeinterface.hpp>
#include <ka/typetraits.hpp>
#include <ka/utility.hpp>
#include <sstream>

namespace qi::detail
{
  template<typename... T>
  qi::Signature tupleSignature(ka::type_t<T...>)
  {
    std::ostringstream signature;
    signature << static_cast<char>(qi::Signature::Type_Tuple);
    (signature << ... << qi::typeOf<ka::RemoveCvRef<T>>()->signature())
        << static_cast<char>(qi::Signature::Type_Tuple_End);
    return qi::Signature(signature.str());
  }

  template<typename T> inline
  qi::Signature functionArgumentsSignature()
  {
    using FnArgs = boost::callable_traits::args_t<T, ka::type_t>;
    return tupleSignature(FnArgs{});
  }
}

#endif  // _QITYPE_DETAIL_FUNCTIONSIGNATURE_HXX_
