/*
** Author(s):
**  - Jean-Charles DELAY <jdelay@aldebaran-robotics.com>
**  - Cedric GESTES      <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef  QI_CORE_RUNNABLE_HPP_
# define QI_CORE_RUNNABLE_HPP_


namespace qi {

  class Runnable
  {
  public:
    virtual void run() = 0;
  };

}

#endif  // QI_CORE_RUNNABLE_HPP_
