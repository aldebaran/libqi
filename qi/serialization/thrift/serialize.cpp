#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#include <iostream>
#include <qi/collections/variables_list.hpp>
#include <qi/messaging/call_definition.hpp>
#include <qi/messaging/result_definition.hpp>
#include <qi/serialization/thrift/serialize.hpp>


namespace qi {
  namespace serialization {

    void thriftSerialize(::apache::thrift::protocol::TProtocol* protocol, const int &t, int field)
    {
      DEBUGOUT_THRIFT_SER(std::cout << "Serialize(int)" << std::endl);
      protocol->writeI32(t);
    }

    void thriftSerialize(::apache::thrift::protocol::TProtocol* protocol, const float &t, int field)
    {
      DEBUGOUT_THRIFT_SER(std::cout << "Serialize(float)" << std::endl);
      //protocol->writeDouble(t);
    }

    void thriftSerialize(::apache::thrift::protocol::TProtocol* protocol, const double &t, int field)
    {
      DEBUGOUT_THRIFT_SER(std::cout << "Serialize(double)" << std::endl);
      protocol->writeDouble(t);
    }

    void thriftSerialize(::apache::thrift::protocol::TProtocol* protocol, const bool &t, int field)
    {
      DEBUGOUT_THRIFT_SER(std::cout << "Serialize(bool)" << std::endl);
    }

    void thriftSerialize(::apache::thrift::protocol::TProtocol* protocol, const std::string &t, int field)
    {
      DEBUGOUT_THRIFT_SER(std::cout << "Serialize(std::string)" << std::endl);
      protocol->writeString(t);
    }
    void thriftSerialize(::apache::thrift::protocol::TProtocol *protocol, const qi::messaging::EmptyValue &t, int field)
    {
      DEBUGOUT_THRIFT_SER(std::cout << "Serialize(EmptyValue)" << std::endl);
    }

    void thriftSerialize(::apache::thrift::protocol::TProtocol *protocol, const qi::messaging::ResultDefinition &t, int field)
    {
      DEBUGOUT_THRIFT_SER(std::cout << "Serialize(ResultDefinition)" << std::endl);
      //protocol->readWriteMessage();
    }

    void thriftSerialize(::apache::thrift::protocol::TProtocol *protocol,const qi::messaging::VariableValue &t, int field)
    {
      DEBUGOUT_THRIFT_SER(std::cout << "Serialize(VariablesValue)" << std::endl);
      protocol->writeStructBegin("VariableValue");
      protocol->writeFieldBegin("which", ::apache::thrift::protocol::T_I32, 1);
      VariableValueSerializeVisitor visitor(protocol);
      t.value().apply_visitor(visitor);
      protocol->writeFieldEnd();
      protocol->writeStructEnd();
      //thriftSerialize(protocol, dynamic_cast<std::list< AL::Messaging::VariableValue >(t));
    }

//    void thriftSerialize(::apache::thrift::protocol::TProtocol *protocol, const AL::Messaging::VariablesList &t, int field)
//    {
//      DEBUGOUT_THRIFT_SER(std::cout << "Serialize(VariablesList)" << std::endl);
//      thriftSerialize(protocol, dynamic_cast<const std::vector< AL::Messaging::VariableValue > &>(t));
//    }

    void thriftSerialize(::apache::thrift::protocol::TProtocol *protocol, const qi::messaging::CallDefinition &t, int field)
    {
      DEBUGOUT_THRIFT_SER(std::cout << "Serialize(CallDefinition)" << std::endl);

      protocol->writeStructBegin("CallDefinition");

      protocol->writeFieldBegin("MethodName", ::apache::thrift::protocol::T_STRING, 2);
      protocol->writeString(t.methodName());
      protocol->writeFieldEnd();

      protocol->writeFieldBegin("Params", ::apache::thrift::protocol::T_STRUCT, 4);
      thriftSerialize(protocol, t.args());
      protocol->writeFieldEnd();

      protocol->writeStructEnd();
    }

  }
}
