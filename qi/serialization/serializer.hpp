#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_SERIALIZATION_SERIALIZER_HPP_
#define _QI_SERIALIZATION_SERIALIZER_HPP_

#include <qi/serialization/message.hpp>
#include <qi/serialization/serialize.hpp>

namespace qi {
  namespace serialization {

    class Serializable;

    enum SerializeAction {
      ACTION_SERIALIZE,
      ACTION_DESERIALIZE
    };

    class Serializer {
    private:
      qi::serialization::SerializeAction _action;
      qi::serialization::Message& _message;
    public:
      Serializer(SerializeAction action, qi::serialization::Message& message);

      void visit(bool& b);

      void visit(char& c);

      void visit(int& i);

      void visit(float& f);

      void visit(std::string& s);

      void visit(Serializable& v);

      template <typename T>
      void visit(std::vector<T>& v);

      template <typename K, typename V>
      void visit(std::map<K, V>& m);
    };
  }
}
#include <qi/serialization/serializer.hxx>

#endif  // _QI_SERIALIZATION_SERIALIZER_HPP_
