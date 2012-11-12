#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QIMESSAGING_DATASTREAM_HPP_
#define _QIMESSAGING_DATASTREAM_HPP_

#include <iostream>
#include <list>
#include <vector>
#include <map>

#include <qi/buffer.hpp>
#include <qi/bufferreader.hpp>
#include <qi/types.hpp>
#include <qi/preproc.hpp>

#include <qitype/signature.hpp>
#include <qitype/type.hpp>
#include <qitype/genericvalue.hpp>

#include <qimessaging/api.hpp>


namespace qi {
  class IDataStreamPrivate;
  class ODataStreamPrivate;

#if 0
#include <iostream>
#define __QI_DEBUG_SERIALIZATION_W(x, extra) {                  \
    std::string sig = qi::signature< x >::value();              \
    std::cout << "write(" << sig << ")" << extra << std::endl;  \
  }

#define __QI_DEBUG_SERIALIZATION_R(x, extra) {                  \
    std::string sig = qi::signature< x >::value();              \
    std::cout << "read (" << sig << ")" << extra << std::endl;  \
  }

#define __QI_DEBUG_SERIALIZATION_CONTAINER_W(x, c) {                    \
    std::string sig = qi::signature< x >::value();                      \
    std::cout << "write(" << sig << ") size: " << c.size() << std::endl; \
  }

#define __QI_DEBUG_SERIALIZATION_CONTAINER_R(x, c) {                    \
    std::string sig = qi::signature< x >::value();                      \
    std::cout << "read (" << sig << ") size: " << c.size() << std::endl; \
  }
#else
# define __QI_DEBUG_SERIALIZATION_W(x, extra)
# define __QI_DEBUG_SERIALIZATION_R(x, extra)
# define __QI_DEBUG_SERIALIZATION_CONTAINER_W(x, c)
# define __QI_DEBUG_SERIALIZATION_CONTAINER_R(x, c)
#endif

  class GenericValuePtr;

  /** This class provides data serialization, using
   * a qi::Buffer as a backend.
   *
   *
   */
  class QIMESSAGING_API IDataStream {
  public:

    explicit IDataStream(const qi::Buffer &buffer);
    ~IDataStream();


    enum Status {
      Status_Ok                     = 0,
      Status_ReadError              = 1,
      Status_ReadPastEnd            = 2,
    };

    IDataStream& operator>>(bool      &b);

    IDataStream& operator>>(char        &c);
    IDataStream& operator>>(signed char &c);
    IDataStream& operator>>(short       &s);
    IDataStream& operator>>(int         &i);
    IDataStream& operator>>(long        &l);
    IDataStream& operator>>(long long   &ll);

    IDataStream& operator>>(unsigned char  &uc);
    IDataStream& operator>>(unsigned short &us);
    IDataStream& operator>>(unsigned int   &ui);
    IDataStream& operator>>(unsigned long  &ul);
    IDataStream& operator>>(unsigned long long &ull);

    IDataStream& operator>>(float    &f);
    IDataStream& operator>>(double   &d);
    IDataStream& operator>>(std::string& i);

    IDataStream& operator>>(qi::GenericValuePtr &value);
    IDataStream& operator>>(qi::Buffer &buffer);

    //read raw data
    size_t read(void *data, size_t len);
    void* read(size_t len);
    Status status() const;
    void setStatus(Status status);
    BufferReader& bufferReader();

  private:
    IDataStreamPrivate *_p;
  };

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

  template<typename T>
  IDataStream& operator>>(IDataStream& in, T& v);

}

#include <qimessaging/details/datastream.hxx>



#endif  // _QIMESSAGING_DATASTREAM_HPP_
