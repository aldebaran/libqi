#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010, 2012 Aldebaran Robotics
*/

#ifndef _QIMESSAGING_SERIALIZATION_SERIALIZE_VALUE_HXX_
#define _QIMESSAGING_SERIALIZATION_SERIALIZE_VALUE_HXX_



namespace qi { namespace serialization {
template <>
struct serialize<qi::Value> {
  static inline void write(qi::DataStream &sd, const qi::Value &val) {
    sd.writeInt(val.type());
    switch(val.type()) {
      case qi::Value::Bool:
        sd.writeString("b");
        sd.writeBool(val._private.data.b);
        return;
      case qi::Value::Char:
        sd.writeString("c");
        sd.writeChar(val._private.data.c);
        return;
      case qi::Value::Int32:
        sd.writeString("i");
        sd.writeInt(val._private.data.i);
        return;
      case Value::UInt32:
      case Value::Int64:
      case Value::UInt64:
        throw SerializationError("not implemented");
        return;
      case qi::Value::Float:
        sd.writeString("f");
        sd.writeFloat(val._private.data.f);
        return;
      case qi::Value::Double:
        sd.writeString("d");
        sd.writeDouble(val._private.data.d);
        return;
      case qi::Value::String:{
        sd.writeString("s");
        sd.writeString(val.constValue< std::string >());
        return;
      }
      case qi::Value::List:{
        sd.writeString("[m]");
        const std::list<qi::Value> &le = val.constValue< std::list<qi::Value> >();
        serialize< std::list<qi::Value> >::write(sd, le);
        return;
      }
      case qi::Value::Vector: {
        sd.writeString("[m]");
        const std::vector<qi::Value> &ve = val.constValue< std::vector<qi::Value> >();
        serialize< std::vector<qi::Value> >::write(sd, ve);
        return;
      }
      case qi::Value::Map: {
        sd.writeString("{sm}");
        const std::map<std::string, qi::Value> &me = val.constValue< std::map<std::string, qi::Value> >();
        serialize< std::map<std::string, qi::Value> >::write(sd, me);
        return;
      }
    };
  }

  static inline void read(qi::DataStream &sd, qi::Value &val) {
    std::string sig;
    int type;
    val.clear();
    sd.readInt(type);
    sd.readString(sig);
    switch(type) {
      case qi::Value::Bool:
        val._private.type = qi::Value::Bool;
        sd.readBool(val._private.data.b);
        return;
      case qi::Value::Char:
        val._private.type = qi::Value::Char;
        sd.readChar(val._private.data.c);
        return;
      case qi::Value::Int32:
        val._private.type = qi::Value::Int32;
        sd.readInt(val._private.data.i);
        return;
      case Value::UInt32:
      case Value::Int64:
      case Value::UInt64:
        throw SerializationError("not implemented");
        return;
      case qi::Value::Float:
        val._private.type = qi::Value::Float;
        sd.readFloat(val._private.data.f);
        return;
      case qi::Value::Double:
        val._private.type = qi::Value::Double;
        sd.readDouble(val._private.data.d);
        return;
      case qi::Value::String:
        val.setType(qi::Value::String);
        sd.readString(val.value<std::string>());
        return;
      case qi::Value::List:
        val.setType(qi::Value::List);
        qi::serialization::serialize< std::list<qi::Value> >::read(sd, val.value< std::list<qi::Value> >());
        return;
      case qi::Value::Vector:
        val.setType(qi::Value::Vector);
        qi::serialization::serialize< std::vector<qi::Value> >::read(sd, val.value< std::vector<qi::Value> >());
        return;
      case qi::Value::Map:
        val.setType(qi::Value::Map);
        qi::serialization::serialize< std::map<std::string, qi::Value> >::read(sd, val.value< std::map<std::string, qi::Value> >());
        return;
    };
  }
};

}}

#endif  // _QIMESSAGING_SERIALIZATION_SERIALIZE_POD_HXX_
