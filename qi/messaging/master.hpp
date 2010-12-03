#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef _QI_MESSAGING_MASTER_HPP_
#define _QI_MESSAGING_MASTER_HPP_

#include <string>
#include <boost/scoped_ptr.hpp>

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
    explicit Master(const std::string& masterAddress = "127.0.0.1:5555");

    /// <summary> Finaliser. </summary>
    virtual ~Master();

    bool isInitialized() const;

  private:
    boost::scoped_ptr<detail::MasterImpl> _impl;
  };

}

#endif  // _QI_MESSAGING_MASTER_HPP_

