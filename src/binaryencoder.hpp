#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_BINARYENCODER_HPP_
#define _SRC_BINARYENCODER_HPP_

#include <boost/function.hpp>

#include <qi/buffer.hpp>

#include <qitype/typeinterface.hpp>
#include <qitype/anyvalue.hpp>
#include <qitype/anyobject.hpp>
#include <qitype/binarycodec.hpp>


namespace qi {
  class AnyReference;
  class BinaryEncoderPrivate;

  /** This class provides data deserialization, using
   * a qi::Buffer as a backend.
   *
   *
   */
  class QITYPE_API BinaryEncoder {
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
    QI_DISALLOW_COPY_AND_ASSIGN(BinaryEncoder);
  };

  template<typename T>
  void BinaryEncoder::write(const T &v)
  {
    //last arguments specified, or VS2010 segfault with an internal error...
    encodeBinary(&buffer(), AnyReference(v), SerializeObjectCallback());
  }
}

#endif  // _SRC_BINARYENCODER_HPP_
