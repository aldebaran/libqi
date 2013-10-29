#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_BINARYDECODER_HPP_
#define _SRC_BINARYDECODER_HPP_

#include <boost/noncopyable.hpp>

#include <qi/buffer.hpp>
#include <qi/bufferreader.hpp>

#include <qitype/typeinterface.hpp>
#include <qitype/anyvalue.hpp>
#include <qitype/anyobject.hpp>

#include <qitype/binarycodec.hpp>

#include <boost/function.hpp>



namespace qi {
  class AnyReference;
  class BinaryDecoderPrivate;
  class BinaryEncoderPrivate;

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
    AnyReference gv = AnyReference::from(v);
    decodeBinary(&bufferReader(), gv);
  }

  /** This class provides data deserialization, using
   * a qi::Buffer as a backend.
   *
   *
   */
  class QITYPE_API BinaryEncoder : private boost::noncopyable {
  public:
    enum Status {
      Status_Ok                     = 0,
      Status_WriteError             = 1,
    };

    /// <summary>Default constructor.</summary>
    /// <param name="data">The data.</param>
    explicit BinaryEncoder(qi::Buffer &buffer);
    ~BinaryEncoder();

    //write raw data without any formatting
    int write(const char *str, size_t len);
    //Write the size as uint32_t, then the data
    void writeString(const char *str, size_t len);
    void write(bool      b);

    void write(char        c);
    void write(signed char c);
    void write(short       i);
    void write(int         i);
    void write(long        l);
    void write(long long   ll);

    void write(unsigned char  uc);
    void write(unsigned short us);
    void write(unsigned int   ui);
    void write(unsigned long  ul);
    void write(unsigned long long ull);

    void write(float  i);
    void write(double i);
    void write(const char *);
    void write(const std::string& i);

    void writeValue(const AnyReference &value, boost::function<void()> recurse = boost::function<void()>());
    void writeRaw(const Buffer &buffer);

    template<typename T>
    void write(const T &v);

    void beginList(qi::uint32_t size, const qi::Signature &elementSignature);
    void endList();
    void beginMap(qi::uint32_t size, const qi::Signature &keySignature, const qi::Signature &valueSignature);
    void endMap();
    void beginTuple(const qi::Signature &signature);
    void endTuple();
    void beginDynamic(const qi::Signature &elementSignature);
    void endDynamic();

    Status status() const;
    void setStatus(Status status);
    static const char* statusToStr(Status status);

    Buffer& buffer();
    std::string& signature();

  private:
    BinaryEncoderPrivate *_p;
  };

  template<typename T>
  void BinaryEncoder::write(const T &v)
  {
    //last arguments specified, or VS2010 segfault with an internal error...
    encodeBinary(&buffer(), AnyReference::from(v), SerializeObjectCallback());
  }
}


#endif  // _SRC_BINARYDECODER_HPP_
