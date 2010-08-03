/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef   	AL_SERIALIZATION_THRIFT_HXX_
# define   	AL_SERIALIZATION_THRIFT_HXX_

namespace AL {
  namespace Serialization {

    template <typename U>
    void thriftSerialize(::apache::thrift::protocol::TProtocol *protocol, const std::vector<U> &t, int field)
    {
      std::cout << "Serialize(std::vector)" << std::endl;
      typename std::vector<U>::const_iterator it;
      protocol->writeListBegin(::apache::thrift::protocol::T_STRUCT, t.size());
      for (it = t.begin(); it != t.end(); ++it)
      {
        protocol->writeStructBegin("NoName");
        thriftSerialize(protocol, *it);
        protocol->writeStructEnd();
      }
      protocol->writeListEnd();
    }

    template<class T>
    void VariableValueSerializeVisitor::operator()(T const & value) const
    {
      thriftSerialize(_protocol, value);
    }

    template <typename U>
    void thriftSerialize(::apache::thrift::protocol::TProtocol *protocol, const std::list<U> &t, int field)
    {
      std::cout << "Serialize(std::list)" << std::endl;
      typename std::list<U>::const_iterator it;
      protocol->writeListBegin(::apache::thrift::protocol::T_STRUCT, t.size());
      for (it = t.begin(); it != t.end(); ++it)
      {
        protocol->writeStructBegin("NoName");
        thriftSerialize(protocol, *it);
        protocol->writeStructEnd();
      }
      protocol->writeListEnd();
    }

  }
}

#endif	    /* !AL_SERIALIZATION_THRIFT_HXX_ */
