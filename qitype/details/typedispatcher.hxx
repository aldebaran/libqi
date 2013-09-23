#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#include <qitype/typeobject.hpp>
#include <qitype/anyobject.hpp>

namespace qi {

  template<typename TypeDispatcher>
  TypeDispatcher& typeDispatch(const TypeDispatcher &vv, AnyReference value)
  {
    if (!value.type)
      throw std::runtime_error("NULL type");
    TypeDispatcher& v = const_cast<TypeDispatcher&>(vv);
    switch(value.kind())
    {
      case TypeKind_Void:
        v.visitVoid();
        break;
      case TypeKind_Unknown:
        v.visitUnknown(value);
        break;
      case TypeKind_Int:
      {
        IntTypeInterface* tint = static_cast<IntTypeInterface*>(value.type);

        v.visitInt(value.toInt(), tint->isSigned(), tint->size());
        break;
      }
      case TypeKind_Float:
      {
        FloatTypeInterface* tfloat = static_cast<FloatTypeInterface*>(value.type);
        v.visitFloat(value.toDouble(), tfloat->size());
        break;
      }
      case TypeKind_String:
      {
        StringTypeInterface* tstring = static_cast<StringTypeInterface*>(value.type);
        std::pair<char*, size_t> content = tstring->get(value.value);
        v.visitString(content.first, content.second);
        break;
      }
      case TypeKind_List:
      {
        v.visitList(value.begin(), value.end());
        break;
      }
      case TypeKind_Map:
      {
        v.visitMap(value.begin(), value.end());
        break;
      }
      case TypeKind_Object:
      {
        v.visitObject(GenericObject(static_cast<ObjectTypeInterface*>(value.type), value.value));
        break;
      }
      case TypeKind_Pointer:
      {
        AnyReference pointee = *value;
        PointerTypeInterface* type = static_cast<PointerTypeInterface*>(value.type);
        if (type->pointerKind() == PointerTypeInterface::Shared
          && pointee.kind() == TypeKind_Object)
        { // shared_ptr<Foo> p with Foo object type.
          // Create our own shared_ptr, that holds p and delete it on destruction
          qiLogDebug("qitype.typedispatcher") << "Detected object shared ptr";
          AnyReference shared_ptr = value.clone();
          AnyObject ao(new GenericObject(static_cast<ObjectTypeInterface*>(pointee.type), pointee.value),
            boost::bind(&AnyObject::deleteCustomDeleter, _1, (boost::function<void(Empty*)>)boost::bind(&AnyReference::destroy, shared_ptr)));
          v.visitAnyObject(ao);
        }
        else
          v.visitPointer(pointee);
        break;
      }
      case TypeKind_Tuple:
      {
        StructTypeInterface* ttuple = static_cast<StructTypeInterface*>(value.type);
        std::vector<AnyReference> tuple = ttuple->values(value.value);
        v.visitTuple(ttuple->className(), tuple, ttuple->elementsName());
        break;
      }
      case TypeKind_Dynamic:
      {
        if (value.type->info() == typeOf<AnyObject>()->info())
        {
          AnyObject* o = (AnyObject*)value.type->ptrFromStorage(&value.value);
          v.visitAnyObject(*o);
        }
        else
          v.visitDynamic(value.asDynamic());
        break;
      }
      case TypeKind_Raw:
      {
        v.visitRaw(value);
        break;
      }
      case TypeKind_Iterator:
      {
        v.visitIterator(value);
        break;
      }
      case TypeKind_Function:
      case TypeKind_Signal:
      case TypeKind_Property:
        qiLogError("qitype.typedispatcher") << "Signal and Property not handled";
    }
    return v;
  }

}
