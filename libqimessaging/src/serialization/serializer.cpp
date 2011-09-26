/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#include <qimessaging/serialization/serializer.hpp>
#include <qimessaging/serialization/serializable.hpp>

namespace qi {
  namespace serialization {

    /// <summary> Constructs a serializer or deserializer with a reference to the message </summary>
    /// <param name="action"> The action: defines if we should serialize or desiralize</param>
    /// <param name="message"> [in,out] The message to serialize to / deserialize from </param>
    Serializer::Serializer(SerializeAction action, qi::Message& message) :
      _action(action), _message(message) {}

    /// <summary> Visits a boolean </summary>
    /// <param name="b"> [in,out] The bool to serialize or deserialize </param>
    void Serializer::visit(bool& b) {
      if (_action == ACTION_SERIALIZE) {
        serialize<bool>::write(_message, b);
      } else {
        serialize<bool>::read(_message, b);
      }
    }

    /// <summary> Visits a char </summary>
    /// <param name="c"> [in,out] The char to to serialize or deserialize </param>
    void Serializer::visit(char& c) {
      if (_action == ACTION_SERIALIZE) {
        serialize<char>::write(_message, c);
      } else {
        serialize<char>::read(_message, c);
      }
    }

    /// <summary> Visits an int </summary>
    /// <param name="i"> [in,out] The int to serialize or deserialize </param>
    void Serializer::visit(int& i) {
      if (_action == ACTION_SERIALIZE) {
        serialize<int>::write(_message, i);
      } else {
        serialize<int>::read(_message, i);
      }
    }

    /// <summary> Visits a float </summary>
    /// <param name="f"> [in,out] The float to serialize or deserialize </param>
    void Serializer::visit(float& f) {
      if (_action == ACTION_SERIALIZE) {
        serialize<float>::write(_message, f);
      } else {
        serialize<float>::read(_message, f);
      }
    }

    /// <summary> Visits a string </summary>
    /// <param name="s"> [in,out] The string to serialize or deserialize </param>
    void Serializer::visit(std::string& s) {
      if (_action == ACTION_SERIALIZE) {
        serialize<std::string>::write(_message, s);
      } else {
        serialize<std::string>::read(_message, s);
      }
    }

    /// <summary> Visits a struct derived from serializable </summary>
    /// <param name="v"> [in,out] The serializable to serialize or deserialize </param>
    void Serializer::visit(Serializable& v) {
      v.serialize(*this);
    }

  }
}

