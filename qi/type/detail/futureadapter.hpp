#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef QI_TYPE_DETAIL_FUTURE_ADAPTER_HPP_
#define QI_TYPE_DETAIL_FUTURE_ADAPTER_HPP_

#include <qi/type/detail/anyreference.hpp>
#include <qi/future.hpp>

namespace qi
{
namespace detail
{

template<typename T> void hold(T) {}

template <typename T>
inline T extractFuture(const qi::Future<qi::AnyReference>& metaFut);

template <typename T>
inline void futureAdapter(const qi::Future<qi::AnyReference>& metaFut, qi::Promise<T> promise);

template <typename T>
inline void futureAdapterVal(const qi::Future<qi::AnyValue>& metaFut, qi::Promise<T> promise);

}
}

#endif
