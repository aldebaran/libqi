#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#include <qitype/typeobject.hpp>
#include <qitype/genericobject.hpp>

namespace qi {

  template<typename TypeDispatcher>
  TypeDispatcher& typeDispatch(const TypeDispatcher &vv, Type *type, void**storage)
  {
    TypeDispatcher& v = const_cast<TypeDispatcher&>(vv);
    switch(type->kind())
    {
      case qi::Type::Void:
        v.visitVoid(type);
        break;
      case qi::Type::Unknown:
        v.visitUnknown(type, storage);
        break;
      case qi::Type::Int:
      {
        TypeInt* tint = static_cast<TypeInt*>(type);
        /* Here we assume that '0' is represented by storage=0 in the byValue case.
        */
        v.visitInt(tint, (storage&&*storage)?tint->get(*storage):0, tint->isSigned(), tint->size());
        break;
      }
      case qi::Type::Float:
      {
        TypeFloat* tfloat = static_cast<TypeFloat*>(type);
        v.visitFloat(tfloat, (storage&&*storage)?tfloat->get(*storage):0, tfloat->size());
        break;
      }
      case qi::Type::String:
      {
        TypeString* tstring = static_cast<TypeString*>(type);
        v.visitString(tstring, *storage);
        break;
      }
      case qi::Type::List:
      {
        TypeList* tlist = static_cast<TypeList*>(type);
        v.visitList(GenericList(tlist, *storage));
        break;
      }
      case qi::Type::Map:
      {
        TypeMap * tlist = static_cast<TypeMap *>(type);
        v.visitMap(GenericMap(tlist, *storage));
        break;
      }
      case qi::Type::Object:
      {
        v.visitObject(GenericObject(static_cast<ObjectType*>(type), *storage));
        break;
      }
      case qi::Type::Pointer:
      {
        TypePointer* tpointer = static_cast<TypePointer*>(type);
        v.visitPointer(tpointer, *storage, (storage&&*storage)?tpointer->dereference(*storage):GenericValue());
        break;
      }
      case qi::Type::Tuple:
      {
        TypeTuple* ttuple = static_cast<TypeTuple*>(type);
        v.visitTuple(ttuple, *storage);
        break;
      }
      case qi::Type::Dynamic:
      {
        std::pair<GenericValue, bool> gv(GenericValue(), false);
        if (storage && *storage)
          gv = static_cast<TypeDynamic*>(type)->get(*storage);
        v.visitDynamic(type, gv.first);
        if (gv.second)
          gv.first.destroy();
      }
    }
    return v;
  }

}
