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

class QI_API Actor
{
public:
  Actor() :
    _strand()
  {}
  Actor(qi::ExecutionContext& ec) :
    _strand(ec)
  {}

  qi::Strand* strand() const
  {
    return &_strand;
  }

private:
  mutable qi::Strand _strand;
};

}

#endif  // _QI_ACTOR_HPP_
