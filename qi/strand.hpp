#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QI_STRAND_HPP_
#define _QI_STRAND_HPP_

#include <qi/detail/executioncontext.hpp>
#include <qi/future.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/function.hpp>

namespace qi
{

class StrandPrivate;

/** Class that schedules tasks sequentially
 *
 * A strand allows one to schedule work on an eventloop with the guaranty
 * that two callback will never be called concurrently.
 *
 * Methods are thread-safe except for destructor which must never be called
 * concurrently.
 */
class QI_API Strand : public ExecutionContext
{
public:
  /// Construct a strand that will schedule work on \p eventLoop
  Strand(qi::ExecutionContext& eventLoop);
  /** Destroys the strand
   *
   * This will wait for all scheduled tasks to finish
   */
  ~Strand();

  void post(const boost::function<void()>& callback);

  qi::Future<void> async(const boost::function<void()>& cb,
      qi::SteadyClockTimePoint tp);
  qi::Future<void> async(const boost::function<void()>& cb,
      qi::Duration delay = qi::Duration(0));

  bool isInThisContext();

private:
  boost::shared_ptr<StrandPrivate> _p;
};

}

#endif  // _QI_STRAND_HPP_
