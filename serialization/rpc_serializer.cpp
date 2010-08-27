/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <alcommon-ng/serialization/rpc_serializer.hpp>
#include <alcommon-ng/serialization/serializer.hpp>

// specific boost serializers for these messages
#include <alcommon-ng/messaging/call_definition_serialization.hxx>
#include <alcommon-ng/messaging/result_definition_serialization.hxx>


namespace AL {
  using Serialization::Serializer;
  namespace Messaging {
    
    std::string RpcSerializer::serializeCall(const CallDefinition& callDef) {
      return Serializer::serialize(callDef);
    }

    std::string RpcSerializer::serializeResult(const ResultDefinition& resultDef) {
      return Serializer::serialize(resultDef);
    }

    CallDefinition RpcSerializer::deserializeCall(const std::string& buffer) {
      return Serializer::deserialize<CallDefinition>(buffer);
    }

    ResultDefinition RpcSerializer::deserializeResult(const std::string& buffer) {
      return Serializer::deserialize<ResultDefinition>(buffer);
    }
  }
}
