/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef     AL_SERIALIZATION_THRIFT_HPP_
# define     AL_SERIALIZATION_THRIFT_HPP_

#include <protocol/TProtocol.h>
#include <boost/variant/static_visitor.hpp>

//#define DEBUGOUT_THRIFT_SER(a) {a;}
#define DEBUGOUT_THRIFT_SER(a)


namespace apache {
  namespace thrift {
    namespace protocol {
      class TProtocol;
    }
  }
}


namespace AL {

  namespace Messaging {
    class ResultDefinition;
    class VariableValue;
    class VariablesList;
    class CallDefinition;
  }

  namespace Serialization {

    //POD Types
    void thriftSerialize(::apache::thrift::protocol::TProtocol* protocol, const int &t, int field = 0);
    void thriftSerialize(::apache::thrift::protocol::TProtocol* protocol, const float &t, int field = 0);
    void thriftSerialize(::apache::thrift::protocol::TProtocol* protocol, const double &t, int field = 0);
    void thriftSerialize(::apache::thrift::protocol::TProtocol* protocol, const bool &t, int field = 0);

    //STL Types
    void thriftSerialize(::apache::thrift::protocol::TProtocol* protocol, const std::string &t, int field = 0);
    template <typename U>
    void thriftSerialize(::apache::thrift::protocol::TProtocol *protocol, const std::vector<U> &t, int field = 0);
    template <typename U>
    void thriftSerialize(::apache::thrift::protocol::TProtocol *protocol, const std::list<U> &t, int field = 0);

    //AL::Messaging Types
    void thriftSerialize(::apache::thrift::protocol::TProtocol *protocol, const AL::Messaging::ResultDefinition &t, int field = 0);
    void thriftSerialize(::apache::thrift::protocol::TProtocol *protocol, const AL::Messaging::VariableValue &t, int field = 0);
    void thriftSerialize(::apache::thrift::protocol::TProtocol *protocol, const AL::Messaging::EmptyValue &t, int field = 0);
    void thriftSerialize(::apache::thrift::protocol::TProtocol *protocol, const AL::Messaging::CallDefinition &t, int field = 0);

    struct VariableValueSerializeVisitor : boost::static_visitor<> {
      VariableValueSerializeVisitor(::apache::thrift::protocol::TProtocol *protocol)
        : _protocol(protocol)
      {}

      template<class T>
      void operator()(T const & value) const;

    private:
      ::apache::thrift::protocol::TProtocol *_protocol;
    };


  }
}

#include <alcommon-ng/serialization/thrift/serialize.hxx>

#endif      /* !AL_SERIALIZATION_THRIFT_HPP_ */
