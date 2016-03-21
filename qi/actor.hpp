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
  mutable qi::Strand _strand; // The name of this members needs to be declared before using it in the signature of
                              // the following member functions. This is a C++ restriction so do not move this down.
public:
  Actor() = default;
  Actor(const Actor&) = delete; // An actor cannot be copy-able nor move-able.

  explicit Actor(qi::ExecutionContext& ec)
    : _strand(ec)
  {}

  virtual ~Actor() = default;

  qi::Strand* strand() const
  {
    return &_strand;
  }

  template<class... Args>
  auto stranded(Args&&... args) const
    -> decltype(_strand.schedulerFor(std::forward<Args>(args)...)) // TODO C++14: remove this line
  {
    return _strand.schedulerFor(std::forward<Args>(args)...);
  }

  template<class... Args>
  auto async(Args&&... args) const
    -> decltype(_strand.async(std::forward<Args>(args)...)) // TODO C++14: remove this line
  {
    return _strand.async(std::forward<Args>(args)...);
  }

  template<class... Args>
  auto asyncDelay(Args&&... args) const
    -> decltype(_strand.asyncDelay(std::forward<Args>(args)...)) // TODO C++14: remove this line
  {
    return _strand.asyncDelay(std::forward<Args>(args)...);
  }

  template<class... Args>
  auto asyncAt(Args&&... args) const
    -> decltype(_strand.asyncAt(std::forward<Args>(args)...)) // TODO C++14: remove this line
  {
    return _strand.asyncAt(std::forward<Args>(args)...);
  }

  void joinTasks()
  {
    _strand.join();
  }

};

}

#endif  // _QI_ACTOR_HPP_
