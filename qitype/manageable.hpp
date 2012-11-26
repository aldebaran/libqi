#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_MANAGEABLE_HPP_
#define _QITYPE_MANAGEABLE_HPP_

#include <qitype/api.hpp>

namespace qi {

  class ManageablePrivate;
  class EventLoop;

/** User classes can inherit from Manageable to benefit from  Event loop
  *  management
  */
  class QITYPE_API Manageable
  {
  public:
    Manageable();
    virtual ~Manageable();
    Manageable(const Manageable& b);
    void operator = (const Manageable& b);

    virtual EventLoop* eventLoop() const;
    virtual void moveToEventLoop(EventLoop* eventLoop);

    ManageablePrivate* _p;
  };
}

#endif  // _QITYPE_MANAGEABLE_HPP_
