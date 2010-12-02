#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef _QI_CORE_RUNNABLE_HPP_
#define _QI_CORE_RUNNABLE_HPP_


namespace qi {

  class Runnable
  {
  public:
    virtual void run() = 0;
  };

}

#endif  // _QI_CORE_RUNNABLE_HPP_
