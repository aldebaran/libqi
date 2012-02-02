#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010, 2011, 2012 Aldebaran Robotics
*/


#ifndef _QIMESSAGING_SERIALIZATION_MESSAGE_HPP_
#define _QIMESSAGING_SERIALIZATION_MESSAGE_HPP_

#include <iostream>
#include <qimessaging/api.hpp>
#include <qimessaging/serialization/datastream.hpp>
//#include <qimessaging/signature.hpp>

namespace qi {

  /// <summary>A serialized message</summary>
  /// \ingroup Serialization
  class QIMESSAGING_API Message {
  public:

    enum MessageType {
      Call,
      Answer,
      Event,
      Error,
      None
    };

    /// <summary>Default constructor. </summary>
    Message()
    {}

    /// <summary>Default constructor.</summary>
    /// <param name="data">The data.</param>
    Message(const std::string &data)
      : _data(data)
    {}


    /// <summary>Gets the string. </summary>
    /// <returns> The string representation of the serialized message</returns>
    std::string str()const {
      return _data;
    }

    /// <summary>Gets the string.</summary>
    /// <param name="str">The result string.</param>
    void str(const std::string &str) {
      _data = str;
    }

  public:
    //TODO: temporary: need cleanup
    MessageType  type;
    unsigned int size;
    std::string  idCaller;
    std::string  idModule;
    std::string  idObject;
    std::string  _data;
  };

}

#endif  // _QIMESSAGING_SERIALIZATION_MESSAGE_HPP_
