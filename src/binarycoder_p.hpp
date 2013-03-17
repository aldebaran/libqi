#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QIMESSAGING_BINARYCODER_P_HPP_
#define _QIMESSAGING_BINARYCODER_P_HPP_


namespace qi {
  class BinaryDecoder;
  class BinaryEncoder;

  class BinaryDecoderPrivate {
    public:
      BinaryDecoderPrivate(qi::BufferReader* buffer);
      ~BinaryDecoderPrivate();

      BinaryDecoder::Status _status;
      BufferReader *_reader;
  };

  class BinaryEncoderPrivate {
    public:
      BinaryEncoderPrivate(qi::Buffer& buffer);
      ~BinaryEncoderPrivate();

      BinaryEncoder::Status _status;
      Buffer _buffer;
      std::string _signature;
      unsigned int _innerSerialization;
  };
}

#endif // _QIMESSAGING_BINARYCODER_P_HPP_
