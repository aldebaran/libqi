/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef   	QIDATASTREAM_H_
# define   	QIDATASTREAM_H_

#include <qimessaging/datastream.hpp>

typedef qi::IDataStream QiIDataStream;
typedef qi::ODataStream QiODataStream;

  template<typename T>
  qi::ODataStream &operator<<(qi::ODataStream &sd, const QList<T> &v) {
    typedef QList<T> _typefordebug;
    typename QList<T>::const_iterator it = v.begin();
    typename QList<T>::const_iterator end = v.end();

    sd << (int)v.size();
    for (; it != end; ++it) {
      sd << *it;
    }
    __QI_DEBUG_SERIALIZATION_CONTAINER_W(_typefordebug, v);
    return sd;
  }

  template<typename T>
  qi::IDataStream &operator>>(qi::IDataStream &sd, QList<T> &v) {
    typedef QList<T> _typefordebug;
    int sz;
    sd >> sz;
    v.clear();
    if (sz) {
      v.resize(sz);
      typename QList<T>::iterator it = v.begin();
      typename QList<T>::iterator end = v.end();
      for (; it != end; ++it) {
        sd >> *it;
      }
    }
    __QI_DEBUG_SERIALIZATION_CONTAINER_R(_typefordebug, v);
    return sd;
  }


  template<typename T>
  qi::ODataStream &operator<<(qi::ODataStream &sd, const QVector<T> &v) {
    typedef QVector<T> _typefordebug;
    typename QVector<T>::const_iterator it = v.begin();
    typename QVector<T>::const_iterator end = v.end();

    sd << (int)v.size();
    for (; it != end; ++it) {
      sd << *it;
    }
    __QI_DEBUG_SERIALIZATION_CONTAINER_W(_typefordebug, v);
    return sd;
  }

  template<typename T>
  qi::IDataStream &operator>>(qi::IDataStream &sd, QVector<T> &v) {
    typedef QVector<T> _typefordebug;
    int sz;
    sd >> sz;
    v.clear();
    if (sz) {
      v.resize(sz);
      typename QVector<T>::iterator it = v.begin();
      typename QVector<T>::iterator end = v.end();
      for (; it != end; ++it) {
        sd >> *it;
      }
    }
    __QI_DEBUG_SERIALIZATION_CONTAINER_R(_typefordebug, v);
    return sd;
  }

  template<typename K, typename V>
  qi::ODataStream &operator<<(qi::ODataStream &sd, const QMap<K, V> &m) {
    typedef  QMap<K,V> _typefordebug;
    typename QMap<K,V>::const_iterator it = m.begin();
    typename QMap<K,V>::const_iterator end = m.end();

    sd << (int)m.size();

    for (; it != end; ++it) {
      sd << it->first;
      sd << it->second;
    }
    __QI_DEBUG_SERIALIZATION_CONTAINER_W(_typefordebug, m);
    return sd;
  }

  template<typename K, typename V>
  qi::IDataStream &operator>>(qi::IDataStream &sd, QMap<K, V>  &m) {
    typedef  QMap<K,V> _typefordebug;
    int sz;
    sd >> sz;
    m.clear();

    for(int i=0; i < sz; ++i) {
      K k;
      V v;
      sd >> k;
      sd >> v;
      m[k] = v;
    }
    __QI_DEBUG_SERIALIZATION_CONTAINER_R(_typefordebug, m);
    return sd;
  };



#endif 	    /* !QIDATASTREAM_H_ */
