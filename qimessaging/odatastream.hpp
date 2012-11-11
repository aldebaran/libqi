#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef QIMESSAGING_ODATASTREAM_HPP
#define QIMESSAGING_ODATASTREAM_HPP

#include <qi/buffer.hpp>

#include <qitype/type.hpp>
#include <qitype/genericvalue.hpp>
#include <qitype/genericobject.hpp>

#include <qimessaging/api.hpp>

namespace qi {
  class GenericValuePtr;
  class ODataStreamPrivate;

  /** This class provides data deserialization, using
   * a qi::Buffer as a backend.
   *
   *
   */
  class QIMESSAGING_API ODataStream {
  public:
    enum Status {
      Status_Ok                     = 0,
      Status_WriteError             = 1,
    };

    /// <summary>Default constructor.</summary>
    /// <param name="data">The data.</param>
    explicit ODataStream(qi::Buffer &buffer);
    ~ODataStream();

    //write raw data without any formatting
    int write(const char *str, size_t len);
    //Write the size as uint32_t, then the data
    void writeString(const char *str, size_t len);
    ODataStream& operator<<(bool      b);

    ODataStream& operator<<(char        c);
    ODataStream& operator<<(signed char c);
    ODataStream& operator<<(short       i);
    ODataStream& operator<<(int         i);
    ODataStream& operator<<(long        l);
    ODataStream& operator<<(long long   ll);

    ODataStream& operator<<(unsigned char  uc);
    ODataStream& operator<<(unsigned short us);
    ODataStream& operator<<(unsigned int   ui);
    ODataStream& operator<<(unsigned long  ul);
    ODataStream& operator<<(unsigned long long ull);

    ODataStream& operator<<(float  i);
    ODataStream& operator<<(double i);
    ODataStream& operator<<(const char *);
    ODataStream& operator<<(const std::string& i);

    ODataStream &operator<<(const GenericValuePtr &value);
    ODataStream &operator<<(const Buffer &buffer);


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
    ODataStreamPrivate *_p;

    //No default CTOR
    ODataStream() {}

  };

  //generic stream operator. (use qi::Type)
  template<typename T>
  ODataStream& operator<<(ODataStream& out, const T &v);

  namespace details {
    QIMESSAGING_API void serialize(GenericValuePtr val, ODataStream& out);
  }

  template<typename T>
  ODataStream& operator<<(ODataStream& out, const T &v) {
    GenericValuePtr value = GenericValuePtr::from(v);
    qi::details::serialize(value, out);
    return out;
  }
}

#endif // QIMESSAGING_ODATASTREAM_HPP
