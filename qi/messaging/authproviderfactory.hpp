#pragma once
/*
**  Copyright (C) 2014 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QI_MESSAGING_AUTHPROVIDERFACTORY_HPP_
#define _QI_MESSAGING_AUTHPROVIDERFACTORY_HPP_

#include <boost/shared_ptr.hpp>

namespace qi {

  class AuthProvider;
  typedef boost::shared_ptr<AuthProvider> AuthProviderPtr;

  class QI_API AuthProviderFactory
  {
  public:
    virtual ~AuthProviderFactory() {}
    virtual AuthProviderPtr newProvider() = 0;
    virtual unsigned int authVersionMajor() { return 1; }
    virtual unsigned int authVersionMinor() { return 0; }
  };

  typedef boost::shared_ptr<AuthProviderFactory> AuthProviderFactoryPtr;

}

#endif
