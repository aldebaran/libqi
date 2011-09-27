#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef _QI_SERIALIZATION_SERIALIZE_QT_HXX_
#define _QI_SERIALIZATION_SERIALIZE_QT_HXX_

#include <QVector>
#include <QList>
#include <QMap>
#include <QString>
#include <qimessaging/serialization/message.hpp>

namespace qi {
  namespace serialization {

    //QI_SIMPLE_SERIALIZER(String, QString);
    //Inline this function => they do nothing, they just call Message method
    //we keep vector/map not inlined at the moment because they take space.
    template <>
    struct serialize<QString>  {
      static inline void write(Message &sd, const QString &val) {
        std::string hack((char *)val.data(), val.size());
        sd.writeString(hack);
      }
      static inline void read(Message &sd, QString &val) {
        std::string hack;
        sd.readString(hack);
        val.setRawData(hack.data(), hack.size());
      }
    };

    template<typename U>
    struct serialize< QVector<U> >  {

      static void write(Message &sd, const QVector<U> &v) {
        sd.writeInt(v.size());
        if (v.size()) {
          // we should find out if the contents is a fixed size type
          // and directly assign the contents if we can
          typename QVector<U>::const_iterator it = v.begin();
          typename QVector<U>::const_iterator end = v.end();
          for (; it != end; ++it) {
            serialize<U>::write(sd, *it);
          }
        }
        __QI_DEBUG_SERIALIZATION_CONTAINER_W(QVector<U>, v);
      }

      // non-const write
      static void write(Message &sd, QVector<U> &v) {
        sd.writeInt(v.size());
        if (v.size()) {
          // we should find out if the contents is a fixed size type
          // and directly assign the contents if we can
          typename QVector<U>::iterator it = v.begin();
          typename QVector<U>::iterator end = v.end();
          for (; it != end; ++it) {
            serialize<U>::write(sd, *it);
          }
        }
        __QI_DEBUG_SERIALIZATION_CONTAINER_W(QVector<U>, v);
      }

      static void read(Message &sd, QVector<U> &v) {
        int sz;
        sd.readInt(sz);
        v.clear();

        if (sz) {
          v.resize(sz);
          typename QVector<U>::iterator it = v.begin();
          typename QVector<U>::iterator end = v.end();
          for (; it != end; ++it) {
            serialize<U>::read(sd, *it);
          }
        }
        __QI_DEBUG_SERIALIZATION_CONTAINER_R(QVector<U>, v);
      }
    };

    template<typename K, typename V>
    struct serialize< QMap<K, V> >  {

      static void write(Message &sd, const QMap<K, V> &m) {
        sd.writeInt(m.size());
        if (m.size()) {
          typename QMap<K, V>::const_iterator it = m.begin();
          typename QMap<K, V>::const_iterator end = m.end();
          for (; it != end; ++it) {
            serialize<K>::write(sd, it->first);
            serialize<V>::write(sd, it->second);
          }
        }
        typedef QMap<K, V> debugMap;
        __QI_DEBUG_SERIALIZATION_CONTAINER_W(debugMap, m);
      }

      // non-const write
      static void write(Message &sd, QMap<K, V> &m) {
        sd.writeInt(m.size());
        if (m.size()) {
          typename QMap<K, V>::iterator it = m.begin();
          typename QMap<K, V>::iterator end = m.end();
          for (; it != end; ++it) {
            serialize<K>::write(sd, it->first);
            serialize<V>::write(sd, it->second);
          }
        }
        typedef QMap<K, V> debugMap;
        __QI_DEBUG_SERIALIZATION_CONTAINER_W(debugMap, m);
      }

      static void read(Message &sd, QMap<K, V>  &m) {
        int sz;
        sd.readInt(sz);
        m.clear();

        if (sz) {
          for(int i=0; i < sz; ++i) {
            K k;
            V v;
            serialize<K>::read(sd, k);
            serialize<V>::read(sd, v);
            m[k] = v;
          }
        }
        typedef QMap<K, V> debugMap;
        __QI_DEBUG_SERIALIZATION_CONTAINER_R(debugMap, m);
      }
    };

  }
}

#endif  // _QI_SERIALIZATION_SERIALIZE_HXX_
