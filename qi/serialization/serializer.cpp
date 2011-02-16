#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#include <qi/serialization/serializer.hpp>
#include <qi/serialization/serializable.hpp>

namespace qi {
  namespace serialization {

    Serializer::Serializer(SerializeAction action, qi::serialization::Message& message) :
      _action(action), _message(message) {}

    void Serializer::visit(bool& b) {
      if (_action == ACTION_SERIALIZE) {
        serialize<bool>::write(_message, b);
      } else {
        serialize<bool>::read(_message, b);
      }
    }


    void Serializer::visit(char& c) {
      if (_action == ACTION_SERIALIZE) {
        serialize<char>::write(_message, c);
      } else {
        serialize<char>::read(_message, c);
      }
    }

    void Serializer::visit(int& i) {
      if (_action == ACTION_SERIALIZE) {
        serialize<int>::write(_message, i);
      } else {
        serialize<int>::read(_message, i);
      }
    }


    void Serializer::visit(float& f) {
      if (_action == ACTION_SERIALIZE) {
        serialize<float>::write(_message, f);
      } else {
        serialize<float>::read(_message, f);
      }
    }

    void Serializer::visit(std::string& s) {
      if (_action == ACTION_SERIALIZE) {
        serialize<std::string>::write(_message, s);
      } else {
        serialize<std::string>::read(_message, s);
      }
    }

    void Serializer::visit(Serializable& v) {
      v.accept(*this);
    }

  }
}

