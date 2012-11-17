#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef QIMESSAGING_BINARYENCODER_HPP
#define QIMESSAGING_BINARYENCODER_HPP

#include <qi/buffer.hpp>

#include <qitype/type.hpp>
#include <qitype/genericvalue.hpp>
#include <qitype/genericobject.hpp>

#include <qimessaging/api.hpp>

namespace qi {
  class GenericValuePtr;
  class BinaryEncoderPrivate;

  /** This class provides data deserialization, using
   * a qi::Buffer as a backend.
   *
   *
   */
  class QIMESSAGING_API BinaryEncoder {
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

    void write(const GenericValuePtr &value);
    void write(const Buffer &buffer);

    template<typename T> void write(const T &v);

    void beginList(qi::uint32_t size, std::string elementSignature);
    void endList();
    void beginMap(qi::uint32_t size, std::string keySignature, std::string valueSignature);
    void endMap();
    void beginTuple(std::string sig);
    void endTuple();

    Status status() const;
    void setStatus(Status status);

    Buffer& buffer();
    std::string& signature();

  private:
    BinaryEncoderPrivate *_p;

    //No default CTOR
    BinaryEncoder() {}

  };

  namespace details {
    QIMESSAGING_API void serialize(GenericValuePtr val, BinaryEncoder& out);
  }

  template<typename T> void BinaryEncoder::write(const T &v)
  {
    GenericValuePtr value = GenericValuePtr::from(v);
    qi::details::serialize(value, *this);
  }
}

#endif // QIMESSAGING_BINARYENCODER_HPP
