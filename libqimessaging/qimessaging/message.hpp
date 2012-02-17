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
      None   = 0,
      Call   = 1,
      Reply  = 2,
      Event  = 3,
      Error  = 4,
    };

    Message();
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

    inline void setService(const std::string &service) { _service = service; }
    inline std::string service() const                 { return _service; }

    inline void setFunction(const std::string &func) { _func = func; }
    inline std::string function() const              { return _func; }

    inline void setData(const std::string &data) { _data = data; }
    inline std::string data() const              { return _data; }


  protected:
    MessageType  _type;
    unsigned int _id;
    std::string  _service;
    std::string  _dest;
    std::string  _func;
    std::string  _data;
  };

  std::ostream& operator<<(std::ostream& os, const qi::Message& msg);
}



#endif  // _QIMESSAGING_SERIALIZATION_MESSAGE_HPP_
