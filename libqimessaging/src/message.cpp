/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <qimessaging/serialization/message.hpp>
#include <qimessaging/datastream.hpp>
#include <iostream>


std::ostream& operator<<(std::ostream& os, const qi::Message& msg) {
  os << "message {"
     << "id=" << msg.id()
     << ", type=" << (char)msg.type() + '0'
     << ", src=" << msg.source()
     << ", dest=" << msg.destination()
     << ", path=" << msg.path()
     << ", data_size=" << msg.data().size()
     << "}";
  return os;
}

namespace qi {

  Message::Message(const std::string &data)
  {
    qi::DataStream ds(data);
    char c;
    int  i;

    ds >> c;
    _type = (MessageType)c;
    ds >> i;
    _id = i;
    ds >> _src;
    ds >> _dest;
    ds >> _path;
    ds >> _data;
  }

  std::string Message::str()const {
    std::string data;

    qi::DataStream ds(data);

    ds << (char)_type;
    ds << (int)_id;
    ds << _src;
    ds << _dest;
    ds << _path;
    ds << _data;
    return ds.str();
  }

}
