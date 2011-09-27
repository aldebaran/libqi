#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_SERIALIZATION_SERIALIZE_STL_HXX_
#define _QI_SERIALIZATION_SERIALIZE_STL_HXX_

namespace qi {
  namespace serialization {

    QI_SIMPLE_SERIALIZER(String, std::string);


    // std::vector serialization
    template<typename U>
    struct serialize< std::vector<U> >  {

      static void write(Message &sd, const std::vector<U> &v) {
        sd.writeInt(v.size());
        if (v.size()) {
          typename std::vector<U>::const_iterator it = v.begin();
          typename std::vector<U>::const_iterator end = v.end();
          for (; it != end; ++it) {
            serialize<U>::write(sd, *it);
          }
        }
        __QI_DEBUG_SERIALIZATION_CONTAINER_W(std::vector<U>, v);
      }

      // non-const write
      static void write(Message &sd, std::vector<U> &v) {
        sd.writeInt(v.size());
        if (v.size()) {
          typename std::vector<U>::iterator it = v.begin();
          typename std::vector<U>::iterator end = v.end();
          for (; it != end; ++it) {
            serialize<U>::write(sd, *it);
          }
        }
        __QI_DEBUG_SERIALIZATION_CONTAINER_W(std::vector<U>, v);
      }

      static void read(Message &sd, std::vector<U> &v) {
        int sz;
        sd.readInt(sz);
        v.clear();

        if (sz) {
          v.resize(sz);
          typename std::vector<U>::iterator it = v.begin();
          typename std::vector<U>::iterator end = v.end();
          for (; it != end; ++it) {
            serialize<U>::read(sd, *it);
          }
        }
        __QI_DEBUG_SERIALIZATION_CONTAINER_R(std::vector<U>, v);
      }
    };

    // std::list serialization
    template<typename U>
    struct serialize< std::list<U> >  {

      static void write(Message &sd, const std::list<U> &v) {
        sd.writeInt(v.size());
        if (v.size()) {
          typename std::list<U>::const_iterator it = v.begin();
          typename std::list<U>::const_iterator end = v.end();
          for (; it != end; ++it) {
            serialize<U>::write(sd, *it);
          }
        }
        __QI_DEBUG_SERIALIZATION_CONTAINER_W(std::list<U>, v);
      }

      // non-const write
      static void write(Message &sd, std::list<U> &v) {
        sd.writeInt(v.size());
        if (v.size()) {
          typename std::list<U>::iterator it = v.begin();
          typename std::list<U>::iterator end = v.end();
          for (; it != end; ++it) {
            serialize<U>::write(sd, *it);
          }
        }
        __QI_DEBUG_SERIALIZATION_CONTAINER_W(std::list<U>, v);
      }

      static void read(Message &sd, std::list<U> &v) {
        int sz;
        sd.readInt(sz);
        v.clear();

        if (sz) {
          v.resize(sz);
          typename std::list<U>::iterator it = v.begin();
          typename std::list<U>::iterator end = v.end();
          for (; it != end; ++it) {
            serialize<U>::read(sd, *it);
          }
        }
        __QI_DEBUG_SERIALIZATION_CONTAINER_R(std::list<U>, v);
      }
    };

    // std::map serialization
    template<typename K, typename V>
    struct serialize< std::map<K, V> >  {

      static void write(Message &sd, const std::map<K, V> &m) {
        sd.writeInt(m.size());
        if (m.size()) {
          typename std::map<K, V>::const_iterator it = m.begin();
          typename std::map<K, V>::const_iterator end = m.end();
          for (; it != end; ++it) {
            serialize<K>::write(sd, it->first);
            serialize<V>::write(sd, it->second);
          }
        }
        typedef std::map<K, V> debugMap;
        __QI_DEBUG_SERIALIZATION_CONTAINER_W(debugMap, m);
      }

      // non-const write
      static void write(Message &sd, std::map<K, V> &m) {
        sd.writeInt(m.size());
        if (m.size()) {
          typename std::map<K, V>::iterator it = m.begin();
          typename std::map<K, V>::iterator end = m.end();
          for (; it != end; ++it) {
            serialize<K>::write(sd, it->first);
            serialize<V>::write(sd, it->second);
          }
        }
        typedef std::map<K, V> debugMap;
        __QI_DEBUG_SERIALIZATION_CONTAINER_W(debugMap, m);
      }

      static void read(Message &sd, std::map<K, V>  &m) {
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
        typedef std::map<K, V> debugMap;
        __QI_DEBUG_SERIALIZATION_CONTAINER_R(debugMap, m);
      }
    };

  }
}

#endif  // _QI_SERIALIZATION_SERIALIZE_HXX_
