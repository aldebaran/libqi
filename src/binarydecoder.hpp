#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_BINARYDECODER_HPP_
#define _SRC_BINARYDECODER_HPP_

#include <qi/buffer.hpp>
#include <qi/bufferreader.hpp>

#include <qitype/type.hpp>
#include <qitype/genericvalue.hpp>
#include <qitype/genericobject.hpp>

#include <qitype/binarycodec.hpp>
#include <qitype/api.hpp>

namespace qi {
  class AnyReference;
  class BinaryDecoderPrivate;

  /** This class provides data serialization, using
   * a qi::Buffer as a backend.
   *
   *
   */
  class QITYPE_API BinaryDecoder {
  public:

    //explicit BinaryDecoder(const qi::Buffer &buffer);
    explicit BinaryDecoder(qi::BufferReader *buffer);
    ~BinaryDecoder();


    enum Status {
      Status_Ok                     = 0,
      Status_ReadError              = 1,
      Status_ReadPastEnd            = 2,
    };

    void read(bool      &b);

    void read(char        &c);
    void read(signed char &c);
    void read(short       &s);
    void read(int         &i);
    void read(long        &l);
    void read(long long   &ll);

    void read(unsigned char  &uc);
    void read(unsigned short &us);
    void read(unsigned int   &ui);
    void read(unsigned long  &ul);
    void read(unsigned long long &ull);

    void read(float    &f);
    void read(double   &d);
    void read(std::string& i);

    void read(qi::AnyReference &value);
    void read(qi::Buffer &buffer);

    template<typename T> void read(T& v);

    //read raw data
    size_t readRaw(void *data, size_t len);
    void* readRaw(size_t len);
    Status status() const;
    void setStatus(Status status);
    static const char* statusToStr(Status status);
    BufferReader& bufferReader();

  private:
    BinaryDecoderPrivate *_p;
  };

  template<typename T>
  void BinaryDecoder::read(T& v)
  {
    AnyReference gv(v);
    decodeBinary(&bufferReader(), gv);
  }
}

#endif  // _SRC_BINARYDECODER_HPP_
