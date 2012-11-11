#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef QIMESSAGING_ODATASTREAM_P_HPP
#define QIMESSAGING_ODATASTREAM_P_HPP

namespace qi {
  class IDataStream;

  class IDataStreamPrivate {
    public:
      IDataStreamPrivate(const qi::Buffer& buffer);
      ~IDataStreamPrivate();

      IDataStream::Status _status;
      BufferReader _reader;
  };
}

#endif // QIMESSAGING_ODATASTREAM_P_HPP
