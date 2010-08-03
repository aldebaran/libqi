/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <iostream>
#include <alcommon-ng/collections/variables_list.hpp>
#include <alcommon-ng/serialization/call_definition.hpp>
#include <alcommon-ng/serialization/result_definition.hpp>
#include <alcommon-ng/serialization/thrift/serialize.hpp>

namespace AL {
  namespace Serialization {

    void serializeItem(::apache::thrift::protocol::TProtocol* protocol, const int &t, int field)
    {
      std::cout << "Serialize(int)" << std::endl;
    }

    void serializeItem(::apache::thrift::protocol::TProtocol* protocol, const float &t, int field)
    {
      std::cout << "Serialize(float)" << std::endl;
    }

    void serializeItem(::apache::thrift::protocol::TProtocol* protocol, const double &t, int field)
    {
      std::cout << "Serialize(double)" << std::endl;
    }

    void serializeItem(::apache::thrift::protocol::TProtocol* protocol, const bool &t, int field)
    {
      std::cout << "Serialize(bool)" << std::endl;
    }

    void serializeItem(::apache::thrift::protocol::TProtocol* protocol, const std::string &t, int field)
    {
      std::cout << "Serialize(std::string)" << std::endl;
    }

    void serializeItem(::apache::thrift::protocol::TProtocol *protocol, const AL::Messaging::ResultDefinition &t, int field)
    {
      std::cout << "Serialize(ResultDefinition)" << std::endl;
      //protocol->readWriteMessage();
    }

    void serializeItem(::apache::thrift::protocol::TProtocol *protocol,const AL::Messaging::VariableValue &t, int field)
    {
      std::cout << "Serialize(VariablesValue)" << std::endl;
      protocol->writeStructBegin("VariableValue");
      protocol->writeFieldBegin("which", ::apache::thrift::protocol::T_I32, 1);
      VariableValueSerializeVisitor visitor(protocol);
      t.value().apply_visitor(visitor);
      protocol->writeFieldEnd();
      protocol->writeStructEnd();
      //serializeItem(protocol, dynamic_cast<std::list< AL::Messaging::VariableValue >(t));
    }

    void serializeItem(::apache::thrift::protocol::TProtocol *protocol, const AL::Messaging::VariablesList &t, int field)
    {
      std::cout << "Serialize(VariablesList)" << std::endl;
      serializeItem(protocol, dynamic_cast<const std::list< AL::Messaging::VariableValue > &>(t));
    }

    void serializeItem(::apache::thrift::protocol::TProtocol *protocol, const AL::Messaging::CallDefinition &t, int field)
    {
      std::cout << "Serialize(CallDefinition)" << std::endl;

      protocol->writeStructBegin("CallDefinition");

      protocol->writeFieldBegin("RequestId", ::apache::thrift::protocol::T_I32, 1);
      protocol->writeI32(t.getRequestId());
      protocol->writeFieldEnd();

      protocol->writeFieldBegin("MethodName", ::apache::thrift::protocol::T_STRING, 2);
      protocol->writeString(t.getMethodName());
      protocol->writeFieldEnd();

      protocol->writeFieldBegin("ModuleName", ::apache::thrift::protocol::T_STRING, 3);
      protocol->writeString(t.getModuleName());
      protocol->writeFieldEnd();

      protocol->writeFieldBegin("Params", ::apache::thrift::protocol::T_STRUCT, 4);
      serializeItem(protocol, t.getParameters());
      protocol->writeFieldEnd();

      protocol->writeFieldBegin("HasRes", ::apache::thrift::protocol::T_BOOL, 5);
      protocol->writeBool(t.getRequestId());
      protocol->writeFieldEnd();

      protocol->writeFieldBegin("isPC", ::apache::thrift::protocol::T_BOOL, 6);
      protocol->writeBool(t.isPCall());
      protocol->writeFieldEnd();

      protocol->writeFieldBegin("Sender", ::apache::thrift::protocol::T_STRING, 7);
      protocol->writeString(t.getSender());
      protocol->writeFieldEnd();

      protocol->writeStructEnd();
    }

  }
}
