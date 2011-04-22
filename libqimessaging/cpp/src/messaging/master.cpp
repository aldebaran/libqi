/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#include <qimessaging/master.hpp>
#include <string>
#include "src/messaging/master_impl.hpp"

namespace qi {

  Master::Master(const std::string& masterAddress, qi::Context* ctx) :
    _impl(new detail::MasterImpl(masterAddress, ctx)) {}

  bool Master::isInitialized() const {
    return _impl->isInitialized();
  }

  void Master::run() {
    _impl->run();
  }

  Master::~Master() {}
}
