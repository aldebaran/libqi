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
      case Type::Void:
        v.visitVoid();
        break;
      case Type::Unknown:
        v.visitUnknown(value);
        break;
      case Type::Int:
      {
        TypeInt* tint = static_cast<TypeInt*>(value.type);

        v.visitInt(value.toInt(), tint->isSigned(), tint->size());
        break;
      }
      case Type::Float:
      {
        TypeFloat* tfloat = static_cast<TypeFloat*>(value.type);
        v.visitFloat(value.toDouble(), tfloat->size());
        break;
      }
      case Type::String:
      {
        TypeString* tstring = static_cast<TypeString*>(value.type);
        std::pair<char*, size_t> content = tstring->get(value.value);
        v.visitString(content.first, content.second);
        break;
      }
      case Type::List:
      {
        v.visitList(value.begin(), value.end());
        break;
      }
      case Type::Map:
      {
        v.visitMap(value.begin(), value.end());
        break;
      }
      case Type::Object:
      {
        v.visitObject(GenericObject(static_cast<ObjectType*>(value.type), value.value));
        break;
      }
      case Type::Pointer:
      {
        GenericValuePtr pointee = *value;
        TypePointer* type = static_cast<TypePointer*>(value.type);
        if (type->pointerKind() == TypePointer::Shared
          && pointee.kind() == Type::Object)
        { // shared_ptr<Foo> p with Foo object type.
          // Create our own shared_ptr, that holds p and delete it on destruction
          qiLogDebug("qitype.typedispatcher") << "Detected object shared ptr";
          GenericValuePtr shared_ptr = value.clone();
          ObjectPtr o(new GenericObject(static_cast<ObjectType*>(pointee.type), pointee.value),
            boost::bind(&GenericValuePtr::destroy, shared_ptr));
          v.visitObjectPtr(o);
        }
        else
          v.visitPointer(pointee);
        break;
      }
      case Type::Tuple:
      {
        TypeTuple* ttuple = static_cast<TypeTuple*>(value.type);
        std::vector<GenericValuePtr> tuple = ttuple->getValues(value.value);
        v.visitTuple(ttuple->className(), tuple, ttuple->annotations());
        break;
      }
      case Type::Dynamic:
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
      case Type::Raw:
      {
        v.visitRaw(value);
        break;
      }
      case Type::Iterator:
      {
        v.visitIterator(value);
        break;
      }
    }
    return v;
  }

}
