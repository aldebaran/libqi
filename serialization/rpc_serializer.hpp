#pragma once
/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/
#ifndef AL_MESSAGING_SERIALIZATION_RPC_SERIALIZER_HPP_
#define AL_MESSAGING_SERIALIZATION_RPC_SERIALIZER_HPP_

#include <string>
#include <alcommon-ng/messaging/call_definition.hpp>
#include <alcommon-ng/messaging/result_definition.hpp>

namespace AL {
  namespace Messaging {
    class RpcSerializer {
    public:

      std::string serializeCall(const CallDefinition& callDef);
      std::string serializeResult(const ResultDefinition& resultDef);

      CallDefinition deserializeCall(const std::string& buffer);
      ResultDefinition deserializeResult(const std::string& buffer);
    };
  }
}

#endif  // AL_MESSAGING_SERIALIZATION_RPC_SERIALIZER_HPP_
