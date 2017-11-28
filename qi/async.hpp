#pragma once
/**
**  Copyright (C) 2012-2016 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QI_ASYNC_HPP_
#define _QI_ASYNC_HPP_

#include <qi/eventloop.hpp>
#include <qi/future.hpp>
#include <qi/strand.hpp>

namespace qi
{
// some required forward declaration
namespace detail
{
  template <typename F>
  inline auto asyncMaybeActor(F&& cb, qi::Duration delay) ->
      typename std::enable_if<detail::IsAsyncBind<F>::value, typename std::decay<decltype(cb())>::type>::type;

  template <typename F>
  inline auto asyncMaybeActor(F&& cb, qi::Duration delay) ->
      typename std::enable_if<!detail::IsAsyncBind<F>::value,
               qi::Future<typename std::decay<decltype(cb())>::type>>::type;

  template <typename F>
  inline auto asyncMaybeActor(F&& cb, qi::SteadyClockTimePoint timepoint) ->
      typename std::enable_if<detail::IsAsyncBind<F>::value, typename std::decay<decltype(cb())>::type>::type;

  template <typename F>
  inline auto asyncMaybeActor(F&& cb, qi::SteadyClockTimePoint timepoint) ->
      typename std::enable_if<!detail::IsAsyncBind<F>::value,
               qi::Future<typename std::decay<decltype(cb())>::type>>::type;
} // detail

template <typename F>
inline auto asyncAt(F&& callback, qi::SteadyClockTimePoint timepoint)
    -> decltype(qi::getEventLoop()->asyncAt(std::forward<F>(callback), timepoint))
{
  return qi::getEventLoop()->asyncAt(std::forward<F>(callback), timepoint);
}

template <typename F>
inline auto asyncDelay(F&& callback, qi::Duration delay)
    -> decltype(detail::asyncMaybeActor(std::forward<F>(callback), delay))
{
  return detail::asyncMaybeActor(std::forward<F>(callback), delay);
}

template <typename F>
inline auto async(F&& callback)
    -> decltype(asyncDelay(std::forward<F>(callback), qi::Duration(0)))
{
  return asyncDelay(std::forward<F>(callback), qi::Duration(0));
}

/// \copydoc qi::EventLoop::async().
/// \deprecated use qi::async with qi::Duration
template<typename R>
QI_API_DEPRECATED_MSG(Use 'asyncDelay' instead)
inline Future<R> async(boost::function<R()> callback, uint64_t usDelay);

template<typename R>
QI_API_DEPRECATED_MSG(Use 'asyncDelay' instead)
inline Future<R> async(boost::function<R()> callback, qi::Duration delay);

template<typename R>
QI_API_DEPRECATED_MSG(Use 'asyncAt' instead)
inline Future<R> async(boost::function<R()> callback, qi::SteadyClockTimePoint timepoint);

template<typename R>
QI_API_DEPRECATED_MSG(Use 'async' without explicit return type template arguement instead)
inline Future<R> async(detail::Function<R()> callback);

#ifdef DOXYGEN
  /// @deprecated since 2.5
  template<typename R, typename Func, typename ArgTrack>
  QI_API_DEPRECATED qi::Future<R> async(const Func& f, const ArgTrack& toTrack, ...);
#else
#define genCall(n, ATYPEDECL, ATYPES, ADECL, AUSE, comma)\
  template <typename R, typename AF, typename ARG0 comma ATYPEDECL>\
  inline QI_API_DEPRECATED Future<R> async(const AF& fun, const ARG0& arg0 comma ADECL, qi::Duration delay = qi::Duration(0))\
\
  template <typename R, typename AF, typename ARG0 comma ATYPEDECL>\
  inline QI_API_DEPRECATED Future<R> async(const AF& fun, const ARG0& arg0 comma ADECL, qi::SteadyClockTimePoint timepoint)\
QI_GEN(genCall)
#undef genCall
#endif

/// Cancels the future when the timeout expires.
///
/// The output future is the same as the input one, to allow functional
/// composition.
template<typename T, typename Duration>
Future<T> cancelOnTimeout(Future<T> fut, Duration timeout);

} // qi

#include <qi/detail/async.hxx>
#endif  // _QI_ASYNC_HPP
