#pragma once
/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/
#ifndef   __QI_MESSAGING_CONTEXT_HPP__
#define   __QI_MESSAGING_CONTEXT_HPP__

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

#endif // __QI_MESSAGING_CONTEXT_HPP__
