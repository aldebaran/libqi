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
#include <stdint.h>
#include <qimessaging/api.hpp>
#include <qimessaging/value.hpp>
#include <qimessaging/buffer.hpp>

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

  class QIMESSAGING_API DataStream {
  public:


    /// <summary>Default constructor.</summary>
    /// <param name="data">The data.</param>
    explicit DataStream(const qi::Buffer &buffer);
    explicit DataStream(qi::Buffer &buffer);

    size_t read(void *data, size_t len);
    void writeString(const char *str, size_t len);

    DataStream& operator<<(bool     b);
    DataStream& operator<<(char     c);
    DataStream& operator<<(int8_t   c);
    DataStream& operator<<(int16_t  s);
    DataStream& operator<<(int32_t  i);
    DataStream& operator<<(int64_t  l);

    DataStream& operator<<(uint8_t  uc);
    DataStream& operator<<(uint16_t us);
    DataStream& operator<<(uint32_t ui);
    DataStream& operator<<(uint64_t ul);
    DataStream& operator<<(float    f);
    DataStream& operator<<(double   d);
    DataStream& operator<<(const char *);
    DataStream& operator<<(const std::string& i);

    DataStream& operator>>(bool     &b);
    DataStream& operator>>(char     &c);

    DataStream& operator>>(int8_t   &c);
    DataStream& operator>>(int16_t  &i);
    DataStream& operator>>(int32_t  &i);
    DataStream& operator>>(int64_t  &l);

    DataStream& operator>>(uint8_t  &uc);
    DataStream& operator>>(uint16_t &us);
    DataStream& operator>>(uint32_t &ui);
    DataStream& operator>>(uint64_t &ul);

    DataStream& operator>>(float  &i);
    DataStream& operator>>(double &i);
    DataStream& operator>>(std::string& i);

    /// <summary>Gets the string. </summary>
    /// <returns> The string representation of the serialized message</returns>
    //void *data() { return _buffer->data(); }

  private:
    qi::Buffer  _buffer;
    bool        _ro;

    /// <summary>Default constructor. </summary>
    DataStream()
    {}

  };


  template<typename T>
  qi::DataStream &operator<<(qi::DataStream &sd, const std::list<T> &v) {
    typedef std::list<T> _typefordebug;
    typename std::list<T>::const_iterator it = v.begin();
    typename std::list<T>::const_iterator end = v.end();

    sd << (uint32_t)v.size();
    for (; it != end; ++it) {
      sd << *it;
    }
    __QI_DEBUG_SERIALIZATION_CONTAINER_W(_typefordebug, v);
    return sd;
  }

  template<typename T>
  qi::DataStream &operator>>(qi::DataStream &sd, std::list<T> &v) {
    typedef std::list<T> _typefordebug;
    uint32_t sz;
    sd >> sz;
    v.clear();
    if (sz) {
      try {
        v.resize(sz);
      } catch (const std::exception &e) {
        qiLogError("qi.DataStream") << "std::list<T> serialization error, could not resize to "
                                    << sz;
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
  qi::DataStream &operator<<(qi::DataStream &sd, const std::vector<T> &v) {
    typedef std::vector<T> _typefordebug;
    typename std::vector<T>::const_iterator it = v.begin();
    typename std::vector<T>::const_iterator end = v.end();

    sd << (uint32_t)v.size();
    for (; it != end; ++it) {
      sd << *it;
    }
    __QI_DEBUG_SERIALIZATION_CONTAINER_W(_typefordebug, v);
    return sd;
  }

  template<typename T>
  qi::DataStream &operator>>(qi::DataStream &sd, std::vector<T> &v) {
    typedef std::vector<T> _typefordebug;
    uint32_t sz = 0;
    sd >> sz;
    v.clear();
    if (sz) {
      try {
        v.resize(sz);
      } catch (const std::exception &e) {
        qiLogError("qi.DataStream") << "std::vector<T> serialization error, could not resize to "
                                    << sz;
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
  qi::DataStream &operator<<(qi::DataStream &sd, const std::map<K, V> &m) {
    typedef  std::map<K,V> _typefordebug;
    typename std::map<K,V>::const_iterator it = m.begin();
    typename std::map<K,V>::const_iterator end = m.end();

    sd << (uint32_t)m.size();

    for (; it != end; ++it) {
      sd << it->first;
      sd << it->second;
    }
    __QI_DEBUG_SERIALIZATION_CONTAINER_W(_typefordebug, m);
    return sd;
  }

  template<typename K, typename V>
  qi::DataStream &operator>>(qi::DataStream &sd, std::map<K, V>  &m) {
    typedef  std::map<K,V> _typefordebug;
    uint32_t sz;
    sd >> sz;
    m.clear();

    for(uint32_t i=0; i < sz; ++i) {
      K k;
      V v;
      sd >> k;
      sd >> v;
      m[k] = v;
    }
    __QI_DEBUG_SERIALIZATION_CONTAINER_R(_typefordebug, m);
    return sd;
  };

  QIMESSAGING_API qi::DataStream &operator>>(qi::DataStream &sd, qi::Value &value);
  QIMESSAGING_API qi::DataStream &operator<<(qi::DataStream &sd, const qi::Value &value);
}

#endif  // _QIMESSAGING_DATASTREAM_HPP_
