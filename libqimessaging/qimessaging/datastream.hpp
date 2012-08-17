/*
 *  Author(s):
 *  - Chris  Kilner <ckilner@aldebaran-robotics.com>
 *  - Cedric Gestes <gestes@aldebaran-robotics.com>
 *
 *  Copyright (C) 2010, 2011, 2012 Aldebaran Robotics
 */


#pragma once
#ifndef _QIMESSAGING_DATASTREAM_HPP_
#define _QIMESSAGING_DATASTREAM_HPP_

#include <iostream>
#include <list>
#include <vector>
#include <map>
#include <qimessaging/api.hpp>
#include <qimessaging/value.hpp>
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

    IDataStream& operator>>(bool     &b);
    IDataStream& operator>>(char     &c);
    IDataStream& operator>>(qi::int8_t   &c);
    IDataStream& operator>>(qi::int16_t  &s);
    IDataStream& operator>>(qi::int32_t  &i);
    IDataStream& operator>>(qi::int64_t  &l);
    IDataStream& operator>>(long  &l);

    IDataStream& operator>>(qi::uint8_t  &uc);
    IDataStream& operator>>(qi::uint16_t &us);
    IDataStream& operator>>(qi::uint32_t &ui);
    IDataStream& operator>>(qi::uint64_t &ul);
    IDataStream& operator>>(unsigned long  &l);

    IDataStream& operator>>(float    &f);
    IDataStream& operator>>(double   &d);
    IDataStream& operator>>(std::string& i);
        //read raw data
    size_t read(void *data, size_t len);
    void* read(size_t len);
    Status status() const { return _status; };
    void setStatus(Status status) { _status = status; }
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
    ODataStream& operator<<(bool     b);
    ODataStream& operator<<(char     c);

    ODataStream& operator<<(qi::int8_t   c);
    ODataStream& operator<<(qi::int16_t  i);
    ODataStream& operator<<(qi::int32_t  i);
    ODataStream& operator<<(qi::int64_t  l);
    ODataStream& operator<<(long);

    ODataStream& operator<<(qi::uint8_t  uc);
    ODataStream& operator<<(qi::uint16_t us);
    ODataStream& operator<<(qi::uint32_t ui);
    ODataStream& operator<<(qi::uint64_t ul);
    ODataStream& operator<<(unsigned long);

    ODataStream& operator<<(float  i);
    ODataStream& operator<<(double i);
    ODataStream& operator<<(const char *);
    ODataStream& operator<<(const std::string& i);

    Status status() const { return _status; };
    void setStatus(Status status) { _status = status; }

  private:
    Status      _status;
    Buffer _buffer;
    /// <summary>Default constructor. </summary>
    ODataStream()
    {}

  };


  QIMESSAGING_API qi::IDataStream &operator>>(qi::IDataStream &stream, qi::Buffer &buffer);

  QIMESSAGING_API qi::ODataStream &operator<<(qi::ODataStream &stream, const qi::Buffer &buffer);

  QIMESSAGING_API qi::SignatureStream &operator&(qi::SignatureStream &os, const qi::Buffer &buffer);


  template<typename T>
  qi::ODataStream &operator<<(qi::ODataStream &sd, const std::list<T> &v) {
    typedef std::list<T> _typefordebug;
    typename std::list<T>::const_iterator it = v.begin();
    typename std::list<T>::const_iterator end = v.end();

    sd << (qi::uint32_t)v.size();
    for (; it != end; ++it) {
      sd << *it;
    }
    __QI_DEBUG_SERIALIZATION_CONTAINER_W(_typefordebug, v);
    return sd;
  }

  template<typename T>
  qi::IDataStream &operator>>(qi::IDataStream &sd, std::list<T> &v) {
    typedef std::list<T> _typefordebug;
    qi::uint32_t sz = 0;
    sd >> sz;
    v.clear();
    if (sz) {
      try {
        v.resize(sz);
      } catch (const std::exception &) {
        qiLogError("qi.DataStream") << "std::list<T> serialization error, could not resize to "
                                    << sz;
        sd.setStatus(IDataStream::Status_ReadError);
        return sd;
      }
      typename std::list<T>::iterator it = v.begin();
      typename std::list<T>::iterator end = v.end();
      for (; it != end; ++it) {
        sd >> *it;
      }
    }
    __QI_DEBUG_SERIALIZATION_CONTAINER_R(_typefordebug, v);
    return sd;
  }


  template<typename T>
  qi::ODataStream &operator<<(qi::ODataStream &sd, const std::vector<T> &v) {
    typedef std::vector<T> _typefordebug;
    typename std::vector<T>::const_iterator it = v.begin();
    typename std::vector<T>::const_iterator end = v.end();

    sd << (qi::uint32_t)v.size();
    for (; it != end; ++it) {
      sd << *it;
    }
    __QI_DEBUG_SERIALIZATION_CONTAINER_W(_typefordebug, v);
    return sd;
  }

  template<typename T>
  qi::IDataStream &operator>>(qi::IDataStream &sd, std::vector<T> &v) {
    typedef std::vector<T> _typefordebug;
    qi::uint32_t sz = 0;
    sd >> sz;
    v.clear();
    if (sz) {
      try {
        v.resize(sz);
      } catch (const std::exception &) {
        qiLogError("qi.DataStream") << "std::vector<T> serialization error, could not resize to "
                                    << sz;
        sd.setStatus(IDataStream::Status_ReadError);
        return sd;
      }
      typename std::vector<T>::iterator it = v.begin();
      typename std::vector<T>::iterator end = v.end();
      for (; it != end; ++it) {
        sd >> *it;
      }
    }
    __QI_DEBUG_SERIALIZATION_CONTAINER_R(_typefordebug, v);
    return sd;
  }

  template<typename K, typename V>
  qi::ODataStream &operator<<(qi::ODataStream &sd, const std::map<K, V> &m) {
    typedef  std::map<K,V> _typefordebug;
    typename std::map<K,V>::const_iterator it = m.begin();
    typename std::map<K,V>::const_iterator end = m.end();

    sd << (qi::uint32_t)m.size();

    for (; it != end; ++it) {
      sd << it->first;
      sd << it->second;
    }
    __QI_DEBUG_SERIALIZATION_CONTAINER_W(_typefordebug, m);
    return sd;
  }

  template<typename K, typename V>
  qi::IDataStream &operator>>(qi::IDataStream &sd, std::map<K, V>  &m) {
    typedef  std::map<K,V> _typefordebug;
    qi::uint32_t sz = 0;
    sd >> sz;
    m.clear();

    for(qi::uint32_t i=0; i < sz; ++i) {
      K k;
      V v;
      sd >> k;
      sd >> v;
      m[k] = v;
    }
    __QI_DEBUG_SERIALIZATION_CONTAINER_R(_typefordebug, m);
    return sd;
  };

  QIMESSAGING_API qi::ODataStream &operator<<(qi::ODataStream &sd, qi::Value &value);
  QIMESSAGING_API qi::IDataStream &operator>>(qi::IDataStream &sd, const qi::Value &value);
  //QIMESSAGING_API qi::SignatureStream &operator>>(qi::SignatureStream &sd, const qi::Value &value);


}

/** Make a class serializable throug {IO}DataStream.
 * Call with the class name and the list of fields. Each field must
 * itself be serializable.
 */
#define QI_DATASTREAM_STRUCT(Cname, ...) \
  QI_DATASTREAM_STRUCT_DECLARE(Cname) \
  __QI_DATASTREAM_STRUCT_IMPLEMENT_(inline, Cname, __VA_ARGS__)

/** Only declare serialization operators
 */
#define QI_DATASTREAM_STRUCT_DECLARE(Cname)   \
  ::qi::ODataStream &operator<<(::qi::ODataStream &sd, const Cname &value); \
  ::qi::IDataStream &operator>>(::qi::IDataStream &sd, Cname &value);

/** Define serialization operators.
 */
#define QI_DATASTREAM_STRUCT_IMPLEMENT(Cname, ...) \
  __QI_DATASTREAM_STRUCT_IMPLEMENT_(/**/, Cname, __VA_ARGS__)

#define __QI_SERIALIZE_FIELD(_, sep, Field) sep value.Field




#define __QI_DATASTREAM_STRUCT_IMPLEMENT_(inl, Cname, ...)                    \
 inl ::qi::ODataStream &operator<<(::qi::ODataStream &sd, const Cname &value)   \
 {                                                                    \
   return sd                                                          \
   QI_VAARGS_APPLY(__QI_SERIALIZE_FIELD, <<, __VA_ARGS__);             \
 }                                                                    \
 inl ::qi::IDataStream &operator>>(::qi::IDataStream &sd, Cname &value) \
 {                                                                    \
   return sd                                                          \
   QI_VAARGS_APPLY(__QI_SERIALIZE_FIELD, >>, __VA_ARGS__);             \
 }

#endif  // _QIMESSAGING_DATASTREAM_HPP_
