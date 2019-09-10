#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QI_TYPE_BINARYCODEC_HPP_
#define _QI_TYPE_BINARYCODEC_HPP_

#include <qi/atomic.hpp>

#include <qi/api.hpp>
#include <boost/function.hpp>
#include <qi/buffer.hpp>
#include <qi/anyvalue.hpp>
#include <qi/type/metaobject.hpp>
#include <qi/objectuid.hpp>
#include <qi/messaging/messagesocket_fwd.hpp>

namespace qi {

  /// Informations passed when serializing an object
  struct ObjectSerializationInfo
  {
    ObjectSerializationInfo()
      : transmitMetaObject(true),
        metaObjectCachedId(notCached)
    {}

    MetaObject   metaObject;
    bool         transmitMetaObject;
    qi::uint32_t metaObjectCachedId;
    qi::uint32_t serviceId;
    qi::uint32_t objectId;
    boost::optional<ObjectUid> objectUid;
    static const qi::uint32_t notCached = 0xFFFFFFFF;
  };

  /// Type of callback invoked by sdeerializer when it encounters an object
  using DeserializeObjectCallback = boost::function<AnyObject(const ObjectSerializationInfo&)>;

  /// Type of callback invoked by serializer when it encounters an object.
  using SerializeObjectCallback = boost::function<ObjectSerializationInfo(const AnyObject&)>;

  template <typename T>
  AnyReference decodeBinary(qi::BufferReader *buf, T* value,
                            DeserializeObjectCallback onObject=DeserializeObjectCallback(),
                            MessageSocketPtr socket = 0
                            );

   /** Encode content of \p gvp into \p buf.
    * @param buf buffer that will be filled with serialized data
    * @param gvp AnyReference to serialize
    * @param onObject callback invoked each time an object is encountered.
    * @param socket connection context
    * @throw std::runtime_error when the encoding fail
    */
  QI_API void encodeBinary(qi::Buffer *buf, const AutoAnyReference &gvp, SerializeObjectCallback onObject=SerializeObjectCallback(), MessageSocketPtr socket=0);


  /** Decode content of \p buf into \p gvp.
   * @param buf buffer with serialized data
   * @param gvp initialized AnyReference of correct type. Will be filled in.
   * @param onObject callback invoked each time an object is encountered.
   * @param socket connection context
   * @return the result of the deserialize type visitor
   *
   * @throw std::runtime_error when the decoding fail
   */
  QI_API AnyReference decodeBinary(qi::BufferReader *buf, AnyReference gvp, DeserializeObjectCallback onObject=DeserializeObjectCallback(), MessageSocketPtr socket = 0);

  template <typename T>
  AnyReference decodeBinary(qi::BufferReader *buf, T* value, DeserializeObjectCallback onObject, MessageSocketPtr socket) {
    return decodeBinary(buf, AnyReference::fromPtr(value), onObject, socket);
  }
}

#endif  // _QITYPE_BINARYCODEC_HPP_
