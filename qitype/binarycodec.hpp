#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_BINARYCODEC_HPP_
#define _QITYPE_BINARYCODEC_HPP_

#include <qitype/api.hpp>
#include <boost/function.hpp>
#include <qi/buffer.hpp>
#include <qi/bufferreader.hpp>
#include <qitype/genericvalue.hpp>
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
  typedef boost::function<ObjectPtr (const ObjectSerializationInfo&)> DeserializeObjectCallback;

  /// Type of callback invoked by serializer when it encounters an object.
  typedef boost::function<ObjectSerializationInfo (ObjectPtr)> SerializeObjectCallback;

  template <typename T>
  void decodeBinary(qi::BufferReader *buf, T* value, DeserializeObjectCallback onObject=DeserializeObjectCallback());

    /** Encode content of \p gvp into \p buf.
  * @param onObject callback invoked each time an object is encountered.
  */
  QITYPE_API void encodeBinary(qi::Buffer *buf, const AutoGenericValuePtr &gvp, SerializeObjectCallback onObject=SerializeObjectCallback());


  /** Decode content of \p buf into \p gvp.
  * @param buf buffer with serialized data
  * @param gvp initialized GenericValuePtr of correct type. Will be filled in.
  * @param onObject callback invoked each time an object is encountered.
  */
  QITYPE_API void decodeBinary(qi::BufferReader *buf, GenericValuePtr gvp, DeserializeObjectCallback onObject=DeserializeObjectCallback());

  template <typename T>
  void decodeBinary(qi::BufferReader *buf, T* value, DeserializeObjectCallback onObject) {
    decodeBinary(buf, GenericValuePtr::fromPtr(value), onObject);
  }

}

#endif  // _QITYPE_BINARYCODEC_HPP_
