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
#include <qimessaging/api.hpp>
#include <qimessaging/buffer.hpp>
#include <qimessaging/bufferreader.hpp>
#include <qimessaging/signature.hpp>
#include <qi/types.hpp>
#include <qi/preproc.hpp>

namespace qi {

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

  class GenericValue;

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

    IDataStream &operator>>(qi::GenericValue &value);
    IDataStream &operator>>(qi::Buffer &buffer);

    template<typename T>
    IDataStream &operator>>(std::list<T> &v);
    template<typename T>
    IDataStream &operator>>(std::vector<T> &v);
    template<typename K, typename V>
    IDataStream &operator>>(std::map<K, V>  &m);

        //read raw data
    size_t read(void *data, size_t len);
    void* read(size_t len);
    Status status() const { return _status; }
    void setStatus(Status status) { _status = status; }
    BufferReader& getBufferReader() { return _reader;}
  private:
    Status _status;
    BufferReader _reader;
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
      Status_WriteError             = 1
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

    ODataStream &operator<<(const GenericValue &value);
    ODataStream &operator<<(const Buffer &buffer);

    template<typename T>
    ODataStream &operator<<(const std::list<T> &v);
    template<typename T>
    ODataStream &operator<<(const std::vector<T> &v);
    template<typename K, typename V>
    ODataStream &operator<<(const std::map<K, V> &m);

    void beginList(qi::uint32_t size, std::string elementSignature);
    void endList();
    void beginMap(qi::uint32_t size, std::string keySignature, std::string valueSignature);
    void endMap();

    Status status() const { return _status; }
    void setStatus(Status status) { _status = status; }

    Buffer& getBuffer() { return _buffer;}
  private:
    Status      _status;
    Buffer _buffer;
    bool        _innerSerialization;

    //No default CTOR
    ODataStream() {}

  };

  QIMESSAGING_API qi::SignatureStream &operator&(qi::SignatureStream &os, const qi::Buffer &buffer);
  QIMESSAGING_API qi::SignatureStream &operator&(qi::SignatureStream &os, const qi::GenericValue &value);

  template<typename T>
  ODataStream &ODataStream::operator<<(const std::list<T> &v) {
    typedef std::list<T> _typefordebug;
    typename std::list<T>::const_iterator it = v.begin();
    typename std::list<T>::const_iterator end = v.end();

    beginList(v.size(), qi::signatureFromType<T>::value());

    bool wasInnerSerialization = _innerSerialization;
    _innerSerialization = true;
    for (; it != end; ++it) {
      *this << *it;
    }
    _innerSerialization = wasInnerSerialization;

    endList();

    __QI_DEBUG_SERIALIZATION_CONTAINER_W(_typefordebug, v);
    return *this;
  }

  template<typename T>
  IDataStream &IDataStream::operator>>(std::list<T> &v) {
    typedef std::list<T> _typefordebug;
    qi::uint32_t sz = 0;
    *this >> sz;
    v.clear();
    if (sz) {
      try {
        v.resize(sz);
      } catch (const std::exception &) {
        qiLogError("qi.DataStream") << "std::list<T> serialization error, could not resize to "
                                    << sz;
        setStatus(IDataStream::Status_ReadError);
        return *this;
      }
      typename std::list<T>::iterator it = v.begin();
      typename std::list<T>::iterator end = v.end();
      for (; it != end; ++it) {
        *this >> *it;
      }
    }
    __QI_DEBUG_SERIALIZATION_CONTAINER_R(_typefordebug, v);
    return *this;
  }

  template<typename T>
  ODataStream &ODataStream::operator<<(const std::vector<T> &v) {
    typedef std::vector<T> _typefordebug;
    typename std::vector<T>::const_iterator it = v.begin();
    typename std::vector<T>::const_iterator end = v.end();

    beginList(v.size(), qi::signatureFromType<T>::value());

    bool wasInnerSerialization = _innerSerialization;
    _innerSerialization = true;
    for (; it != end; ++it) {
      *this << *it;
    }
    _innerSerialization = wasInnerSerialization;

    endList();

    __QI_DEBUG_SERIALIZATION_CONTAINER_W(_typefordebug, v);
    return *this;
  }

  template<typename T>
  IDataStream &IDataStream::operator>>(std::vector<T> &v) {
    typedef std::vector<T> _typefordebug;
    qi::uint32_t sz = 0;
    *this >> sz;
    v.clear();
    if (sz) {
      try {
        v.resize(sz);
      } catch (const std::exception &) {
        qiLogError("qi.DataStream") << "std::vector<T> serialization error, could not resize to "
                                    << sz;
        setStatus(IDataStream::Status_ReadError);
        return *this;
      }
      typename std::vector<T>::iterator it = v.begin();
      typename std::vector<T>::iterator end = v.end();
      for (; it != end; ++it) {
        *this >> *it;
      }
    }
    __QI_DEBUG_SERIALIZATION_CONTAINER_R(_typefordebug, v);
    return *this;
  }

  template<typename K, typename V>
  ODataStream &ODataStream::operator<<(const std::map<K, V> &m) {
    typedef  std::map<K,V> _typefordebug;
    typename std::map<K,V>::const_iterator it = m.begin();
    typename std::map<K,V>::const_iterator end = m.end();

    beginMap(m.size(), qi::signatureFromType<K>::value(), qi::signatureFromType<V>::value());

    bool wasInnerSerialization = _innerSerialization;
    _innerSerialization = true;
    for (; it != end; ++it) {
      *this << it->first;
      *this << it->second;
    }
    _innerSerialization = wasInnerSerialization;

    endMap();

    __QI_DEBUG_SERIALIZATION_CONTAINER_W(_typefordebug, m);
    return *this;
  }

  template<typename K, typename V>
  IDataStream &IDataStream::operator>>(std::map<K, V>  &m) {
    typedef  std::map<K,V> _typefordebug;
    qi::uint32_t sz = 0;
    *this >> sz;
    m.clear();

    for(qi::uint32_t i=0; i < sz; ++i) {
      K k;
      V v;
      *this >> k;
      *this >> v;
      m[k] = v;
    }
    __QI_DEBUG_SERIALIZATION_CONTAINER_R(_typefordebug, m);
    return *this;
  }
}

/** Make a class serializable throug {IO}DataStream.
 * Call with the class name and the list of fields. Each field must
 * itself be serializable.
 */
#define QI_DATASTREAM_STRUCT(Cname, ...)                        \
  QI_DATASTREAM_STRUCT_DECLARE(Cname)                           \
  __QI_DATASTREAM_STRUCT_IMPLEMENT_(inline, Cname, __VA_ARGS__)

/** Only declare serialization operators
 */
#define QI_DATASTREAM_STRUCT_DECLARE(Cname)                                 \
  ::qi::ODataStream &operator<<(::qi::ODataStream &sd, const Cname &value); \
  ::qi::IDataStream &operator>>(::qi::IDataStream &sd, Cname &value);

/** Define serialization operators.
 */
#define QI_DATASTREAM_STRUCT_IMPLEMENT(Cname, ...)               \
  __QI_DATASTREAM_STRUCT_IMPLEMENT_(/**/, Cname, __VA_ARGS__)

/** Only declare serialization operators
 */
#define QI_DATASTREAM_STRUCT_PRIVATE_ACCESS(Cname)                                 \
  friend ::qi::ODataStream &operator<<(::qi::ODataStream &sd, const Cname &value); \
  friend ::qi::IDataStream &operator>>(::qi::IDataStream &sd, Cname &value);


#define __QI_SERIALIZE_FIELD(_, sep, Field) sep value.Field

#define __QI_DATASTREAM_STRUCT_IMPLEMENT_(inl, Cname, ...)                      \
 inl ::qi::ODataStream &operator<<(::qi::ODataStream &sd, const Cname &value)   \
 {                                                                              \
   return sd                                                                    \
   QI_VAARGS_APPLY(__QI_SERIALIZE_FIELD, <<, __VA_ARGS__);                      \
 }                                                                              \
 inl ::qi::IDataStream &operator>>(::qi::IDataStream &sd, Cname &value)         \
 {                                                                              \
   return sd                                                                    \
   QI_VAARGS_APPLY(__QI_SERIALIZE_FIELD, >>, __VA_ARGS__);                      \
 }

#endif  // _QIMESSAGING_DATASTREAM_HPP_
