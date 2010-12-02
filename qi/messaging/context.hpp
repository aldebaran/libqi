#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef _QI_MESSAGING_CONTEXT_HPP_
#define _QI_MESSAGING_CONTEXT_HPP_

#include <string>

namespace qi {
  class Context {
  public:
    Context();
    Context(const Context& rhs);

    virtual ~Context();
    const std::string& getID() const;

  protected:
    std::string _contextID;
  };
}

#endif  // _QI_MESSAGING_CONTEXT_HPP_
