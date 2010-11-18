/*
** Author(s):
**  - Jean-Charles DELAY <jdelay@aldebaran-robotics.com>
**  - Cedric GESTES      <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef   __QI_CORE_RUNNABLE_HPP__
#define   __QI_CORE_RUNNABLE_HPP__


namespace qi {

  class Runnable
  {
  public:
    virtual void run() = 0;
  };

}

#endif // __QI_CORE_RUNNABLE_HPP__
