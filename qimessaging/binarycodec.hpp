#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef QIMESSAGING_BINARYCODEC_HPP
#define QIMESSAGING_BINARYCODEC_HPP

#include <qimessaging/api.hpp>
#include <boost/function.hpp>
#include <qi/buffer.hpp>
#include <qi/bufferreader.hpp>
#include <qitype/genericvalue.hpp>

namespace qi {

  //generic binary codec API
  QIMESSAGING_API void encodeBinary(qi::Buffer *buf, const qi::GenericValuePtr &gvp);
  QIMESSAGING_API void decodeBinary(qi::BufferReader *buf, qi::GenericValuePtr *gvp);

  template <typename T>
  void encodeBinary(qi::Buffer *buf, const T &t) {
    encodeBinary(buf, qi::GenericValuePtr(&t));
  }

  template <typename T>
  void decodeBinary(qi::BufferReader *buf, T* value) {
    qi::GenericValuePtr gvp(value);
    decodeBinary(buf, &gvp);
  }

}

#endif // QIMESSAGING_BINARYENCODER_HPP
