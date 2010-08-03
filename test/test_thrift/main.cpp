/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Cedric GESTES
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include <alippc/ippc.hpp>
#include <Thrift.h>
#include <protocol/TProtocol.h>
#include <transport/TTransportUtils.h>
#include <protocol/TJSONProtocol.h>

namespace AL {

  namespace Serialization {

    void serializeItem(::apache::thrift::protocol::TProtocol* protocol, const int &t, int field = 0);
    void serializeItem(::apache::thrift::protocol::TProtocol* protocol, const float &t, int field = 0);
    void serializeItem(::apache::thrift::protocol::TProtocol* protocol, const double &t, int field = 0);
    void serializeItem(::apache::thrift::protocol::TProtocol* protocol, const bool &t, int field = 0);
    void serializeItem(::apache::thrift::protocol::TProtocol* protocol, const std::string &t, int field = 0);
    void serializeItem(::apache::thrift::protocol::TProtocol *protocol, const AL::Messaging::ResultDefinition &t, int field = 0);
    template <typename U>
    void serializeItem(::apache::thrift::protocol::TProtocol *protocol, const std::vector<U> &t, int field = 0);
    void serializeItem(::apache::thrift::protocol::TProtocol *protocol,const AL::Messaging::VariableValue &t, int field = 0);
    template <typename U>
    void serializeItem(::apache::thrift::protocol::TProtocol *protocol, const std::list<U> &t, int field = 0);
    void serializeItem(::apache::thrift::protocol::TProtocol *protocol, const AL::Messaging::VariablesList &t, int field = 0);
    void serializeItem(::apache::thrift::protocol::TProtocol *protocol, const AL::Messaging::CallDefinition &t, int field = 0);


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

    template <typename U>
    void serializeItem(::apache::thrift::protocol::TProtocol *protocol, const std::vector<U> &t, int field)
    {
      std::cout << "Serialize(std::vector)" << std::endl;
      typename std::vector<U>::const_iterator it;
      protocol->writeListBegin(::apache::thrift::protocol::T_STRUCT, t.size());
      for (it = t.begin(); it != t.end(); ++it)
      {
        protocol->writeStructBegin("NoName");
        serializeItem(protocol, *it);
        protocol->writeStructEnd();
      }
      protocol->writeListEnd();
    }

    struct VariableValueSerializeVisitor : boost::static_visitor<>
    {
      VariableValueSerializeVisitor(::apache::thrift::protocol::TProtocol *protocol)
        : _protocol(protocol)
      {}

      template<class T>
      void operator()(T const & value) const
      {
        serializeItem(_protocol, value);
      }

    private:
      ::apache::thrift::protocol::TProtocol *_protocol;
    };


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

    template <typename U>
    void serializeItem(::apache::thrift::protocol::TProtocol *protocol, const std::list<U> &t, int field)
    {
      std::cout << "Serialize(std::list)" << std::endl;
      typename std::list<U>::const_iterator it;
      protocol->writeListBegin(::apache::thrift::protocol::T_STRUCT, t.size());
      for (it = t.begin(); it != t.end(); ++it)
      {
        protocol->writeStructBegin("NoName");
        serializeItem(protocol, *it);
        protocol->writeStructEnd();
      }
      protocol->writeListEnd();
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

    template <typename T>
    std::string serializeJsonThrift(T &t)
    {
      using namespace apache::thrift::transport;
      using namespace apache::thrift::protocol;
      TMemoryBuffer* buffer = new TMemoryBuffer;
      boost::shared_ptr<TTransport> trans(buffer);
      TJSONProtocol protocol(trans);

      serializeItem(&protocol, t);

      uint8_t* buf;
      uint32_t size;
      buffer->getBuffer(&buf, &size);
      return std::string((char*)buf, (unsigned int)size);
    }
    }
  }

int main(int argc, char *argv[])
{
  AL::Messaging::CallDefinition      def;

  def.setMethodName("test2");
  def.setSender("titi caca");
  def.push(41);
  //AL::ALPtr<AL::Messaging::ResultDefinition> res;
  std::cout << "Start Serialization" << std::endl;
  std::string json = AL::Serialization::serializeJsonThrift(def);
  std::cout << json << std::endl;
  return 0;
}
