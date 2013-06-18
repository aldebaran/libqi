#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#include <qitype/typeobject.hpp>
#include <qitype/genericobject.hpp>

namespace qi {

  template<typename TypeDispatcher>
  TypeDispatcher& typeDispatch(const TypeDispatcher &vv, GenericValuePtr value)
  {
    if (!value.type)
      throw std::runtime_error("NULL type");
    TypeDispatcher& v = const_cast<TypeDispatcher&>(vv);
    switch(value.kind())
    {
      case TypeInterface::Void:
        v.visitVoid();
        break;
      case TypeInterface::Unknown:
        v.visitUnknown(value);
        break;
      case TypeInterface::Int:
      {
        IntTypeInterface* tint = static_cast<IntTypeInterface*>(value.type);

        v.visitInt(value.toInt(), tint->isSigned(), tint->size());
        break;
      }
      case TypeInterface::Float:
      {
        FloatTypeInterface* tfloat = static_cast<FloatTypeInterface*>(value.type);
        v.visitFloat(value.toDouble(), tfloat->size());
        break;
      }
      case TypeInterface::String:
      {
        StringTypeInterface* tstring = static_cast<StringTypeInterface*>(value.type);
        std::pair<char*, size_t> content = tstring->get(value.value);
        v.visitString(content.first, content.second);
        break;
      }
      case TypeInterface::List:
      {
        v.visitList(value.begin(), value.end());
        break;
      }
      case TypeInterface::Map:
      {
        v.visitMap(value.begin(), value.end());
        break;
      }
      case TypeInterface::Object:
      {
        v.visitObject(GenericObject(static_cast<ObjectTypeInterface*>(value.type), value.value));
        break;
      }
      case TypeInterface::Pointer:
      {
        GenericValuePtr pointee = *value;
        PointerTypeInterface* type = static_cast<PointerTypeInterface*>(value.type);
        if (type->pointerKind() == PointerTypeInterface::Shared
          && pointee.kind() == TypeInterface::Object)
        { // shared_ptr<Foo> p with Foo object type.
          // Create our own shared_ptr, that holds p and delete it on destruction
          qiLogDebug("qitype.typedispatcher") << "Detected object shared ptr";
          GenericValuePtr shared_ptr = value.clone();
          ObjectPtr o(new GenericObject(static_cast<ObjectTypeInterface*>(pointee.type), pointee.value),
            boost::bind(&GenericValuePtr::destroy, shared_ptr));
          v.visitObjectPtr(o);
        }
        else
          v.visitPointer(pointee);
        break;
      }
      case TypeInterface::Tuple:
      {
        StructTypeInterface* ttuple = static_cast<StructTypeInterface*>(value.type);
        std::vector<GenericValuePtr> tuple = ttuple->values(value.value);
        v.visitTuple(ttuple->className(), tuple, ttuple->elementsName());
        break;
      }
      case TypeInterface::Dynamic:
      {
        if (value.type->info() == typeOf<ObjectPtr>()->info())
        {
          ObjectPtr* o = (ObjectPtr*)value.type->ptrFromStorage(&value.value);
          v.visitObjectPtr(*o);
        }
        else
          v.visitDynamic(value.asDynamic());
        break;
      }
      case TypeInterface::Raw:
      {
        v.visitRaw(value);
        break;
      }
      case TypeInterface::Iterator:
      {
        v.visitIterator(value);
        break;
      }
    }
    return v;
  }

}
