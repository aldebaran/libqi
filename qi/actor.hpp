#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QI_ACTOR_HPP_
#define _QI_ACTOR_HPP_

#include <qi/strand.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/function.hpp>

namespace qi
{

/** Class that represents an actor.
 *
 * Inherit from this class if you want your class to be an actor (as in the
 * actor model). This means that your class will receive "messages" and not be
 * called. In other words, there will never be to calls to your object in
 * parallel, they will be queued.
 *
 * \includename{qi/actor.hpp}
 */
class QI_API Actor
{
public:
  Actor() = default;
  Actor(const Actor&) = delete; // An actor cannot be copy-able nor move-able.

  explicit Actor(qi::ExecutionContext& ec)
    : _strand(ec)
  {}

  qi::Strand* strand() const
  {
    return &_strand;
  }

  template<class... Callable>
  auto stranded(Callable&&... callable) const
    -> decltype(strand()->schedulerFor(std::forward<Callable>(callable)...)) // TODO C++14: remove this line
  {
    return _strand.schedulerFor(std::forward<Callable>(callable)...);
  }

private:
  mutable qi::Strand _strand;
};

}

#endif  // _QI_ACTOR_HPP_
