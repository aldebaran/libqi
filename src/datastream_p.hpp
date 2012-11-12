#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QIMESSAGING_DATASTREAM_P_HPP_
#define _QIMESSAGING_DATASTREAM_P_HPP_


namespace qi {
  class IDataStream;
  class ODataStream;

  class IDataStreamPrivate {
    public:
      IDataStreamPrivate(const qi::Buffer& buffer);
      ~IDataStreamPrivate();

      IDataStream::Status _status;
      BufferReader _reader;
  };

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

#endif // _QIMESSAGING_DATASTREAM_P_HPP_
