#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef QITYPE_BINARYENCODER_HPP
#define QITYPE_BINARYENCODER_HPP

#include <boost/function.hpp>

#include <qi/buffer.hpp>

#include <qitype/type.hpp>
#include <qitype/genericvalue.hpp>
#include <qitype/genericobject.hpp>
#include <qitype/binarycodec.hpp>


namespace qi {
  class GenericValuePtr;
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

    void writeValue(const GenericValuePtr &value, boost::function<void()> recurse = boost::function<void()>());
    void writeRaw(const Buffer &buffer);

    template<typename T> void write(const T &v);

    void beginList(qi::uint32_t size, const std::string &elementSignature);
    void endList();
    void beginMap(qi::uint32_t size, const std::string &keySignature, const std::string &valueSignature);
    void endMap();
    void beginTuple(std::string sig);
    void endTuple();
    void beginDynamic(const std::string &elementSignature);
    void endDynamic();

    Status status() const;
    void setStatus(Status status);
    static const char* statusToStr(Status status);

    Buffer& buffer();
    std::string& signature();

  private:
    BinaryEncoderPrivate *_p;

    //No default CTOR
    BinaryEncoder() {}

  };

  template<typename T> void BinaryEncoder::write(const T &v)
  {
    encodeBinary(&buffer(), GenericValueRef(v));
  }
}

#endif // QIMESSAGING_BINARYENCODER_HPP
