#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_BINARYCODEC_HPP_
#define _QITYPE_BINARYCODEC_HPP_

#include <qitype/api.hpp>
#include <boost/function.hpp>
#include <qi/buffer.hpp>
#include <qi/bufferreader.hpp>
#include <qitype/anyvalue.hpp>
#include <qitype/metaobject.hpp>

namespace qi {

  /// Informations passed when serializing an object
  struct ObjectSerializationInfo
  {
    MetaObject metaObject;
    unsigned int serviceId;
    unsigned int objectId;
  };

  /// Type of callback invoked by sdeerializer when it encounters an object
  typedef boost::function<AnyObject (const ObjectSerializationInfo&)> DeserializeObjectCallback;

  /// Type of callback invoked by serializer when it encounters an object.
  typedef boost::function<ObjectSerializationInfo (AnyObject)> SerializeObjectCallback;

  template <typename T>
  void decodeBinary(qi::BufferReader *buf, T* value, DeserializeObjectCallback onObject=DeserializeObjectCallback());

    /** Encode content of \p gvp into \p buf.
  * @param onObject callback invoked each time an object is encountered.
  */
  QITYPE_API void encodeBinary(qi::Buffer *buf, const AutoAnyReference &gvp, SerializeObjectCallback onObject=SerializeObjectCallback());


  /** Decode content of \p buf into \p gvp.
  * @param buf buffer with serialized data
  * @param gvp initialized AnyReference of correct type. Will be filled in.
  * @param onObject callback invoked each time an object is encountered.
  */
  QITYPE_API void decodeBinary(qi::BufferReader *buf, AnyReference gvp, DeserializeObjectCallback onObject=DeserializeObjectCallback());

  template <typename T>
  void decodeBinary(qi::BufferReader *buf, T* value, DeserializeObjectCallback onObject) {
    decodeBinary(buf, AnyReference::fromPtr(value), onObject);
  }




}

#endif  // _QITYPE_BINARYCODEC_HPP_
