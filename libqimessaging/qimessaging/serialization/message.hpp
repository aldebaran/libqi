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

namespace qi {

  /** \class qi::Message
    * This class represent a network message
    */
  class QIMESSAGING_API Message {
  public:

    enum MessageType {
      Call,
      Answer,
      Event,
      Error,
      None
    };

    Message()
    {}

    Message(const std::string &data);

    std::string str()const;

    /// <summary>Gets the string.</summary>
    /// <param name="str">The result string.</param>
    void str(const std::string &str) {
      _data = str;
    }

    inline void setType(MessageType type) { _type = type; }
    inline MessageType type()  const      { return _type; }

    inline void setId(unsigned int id)    { _id = id; }
    inline unsigned int id() const        { return _id; }

    inline void setSource(const std::string &src) { _src = src; }
    inline std::string source() const             { return _src; }

    inline void setDestination(const std::string &dest) { _dest = dest; }
    inline std::string destination() const              { return _dest; }

    inline void setPath(const std::string &path) { _path = path; }
    inline std::string path() const              { return _path; }

    inline void setData(const std::string &data) { _data = data; }
    inline std::string data() const              { return _data; }


  protected:
    MessageType  _type;
    unsigned int _id;
    std::string  _src;
    std::string  _dest;
    std::string  _path;
    std::string  _data;
  };

}

std::ostream& operator<<(std::ostream& os, const qi::Message& msg);


#endif  // _QIMESSAGING_SERIALIZATION_MESSAGE_HPP_
