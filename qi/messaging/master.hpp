#pragma once
/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/
#ifndef QI_NODE_MASTER_NODE_HPP_
#define QI_NODE_MASTER_NODE_HPP_

#include <string>
#include <memory>

namespace qi {
  namespace detail {
    class MasterImpl;
  }

  /// <summary> Master. </summary>
  class Master {
  public:

    /// <summary> Constructor. </summary>
    /// <param name="masterAddress">
    /// The master address.
    /// e.g. 127.0.0.1:5555
    /// </param>
    explicit Master(const std::string& masterAddress);

    /// <summary> Finaliser. </summary>
    virtual ~Master();

  private:
    std::auto_ptr<detail::MasterImpl> fImp;
  };

}

#endif  // QI_NODE_MASTER_NODE_HPP_

