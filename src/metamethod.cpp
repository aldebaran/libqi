/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include <qimessaging/metamethod.hpp>
#include <qimessaging/datastream.hpp>

namespace qi {

  MetaMethod::MetaMethod(unsigned int uid,
                         const std::string& sigret,
                         const std::string& signature)
  : _uid(uid)
  , _signature(signature)
  , _sigreturn(sigret)
  {}

  std::string MetaMethod::signature() const
  {
    return _signature;
  }

  std::string MetaMethod::sigreturn() const
  {
    return _sigreturn;
  }

  unsigned int       MetaMethod::uid() const
  {
    return _uid;
  }

};
