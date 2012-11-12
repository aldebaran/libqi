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
    BinaryEncoder& operator<<(bool      b);

    BinaryEncoder& operator<<(char        c);
    BinaryEncoder& operator<<(signed char c);
    BinaryEncoder& operator<<(short       i);
    BinaryEncoder& operator<<(int         i);
    BinaryEncoder& operator<<(long        l);
    BinaryEncoder& operator<<(long long   ll);

    BinaryEncoder& operator<<(unsigned char  uc);
    BinaryEncoder& operator<<(unsigned short us);
    BinaryEncoder& operator<<(unsigned int   ui);
    BinaryEncoder& operator<<(unsigned long  ul);
    BinaryEncoder& operator<<(unsigned long long ull);

    BinaryEncoder& operator<<(float  i);
    BinaryEncoder& operator<<(double i);
    BinaryEncoder& operator<<(const char *);
    BinaryEncoder& operator<<(const std::string& i);

    BinaryEncoder &operator<<(const GenericValuePtr &value);
    BinaryEncoder &operator<<(const Buffer &buffer);


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

  //generic stream operator. (use qi::Type)
  template<typename T>
  BinaryEncoder& operator<<(BinaryEncoder& out, const T &v);

  namespace details {
    QIMESSAGING_API void serialize(GenericValuePtr val, BinaryEncoder& out);
  }

  template<typename T>
  BinaryEncoder& operator<<(BinaryEncoder& out, const T &v) {
    GenericValuePtr value = GenericValuePtr::from(v);
    qi::details::serialize(value, out);
    return out;
  }
}

#endif // QIMESSAGING_BINARYENCODER_HPP
