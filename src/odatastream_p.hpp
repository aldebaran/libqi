#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef QIMESSAGING_IDATASTREAM_P_HPP
#define QIMESSAGING_IDATASTREAM_P_HPP

namespace qi {
  class ODataStream;

  class ODataStreamPrivate {
    public:
      ODataStreamPrivate(qi::Buffer& buffer);
      ~ODataStreamPrivate();

      ODataStream::Status _status;
      Buffer _buffer;
      std::string _signature;
      unsigned int _innerSerialization;
  };
}

#endif // QIMESSAGING_IDATASTREAM_P_HPP
