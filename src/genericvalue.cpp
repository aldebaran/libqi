/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#include <qitype/genericvalue.hpp>
#include <qitype/genericobject.hpp>

namespace qi
{
  std::pair<GenericValuePtr, bool> GenericValuePtr::convert(Type* targetType) const
  {
    /* Can have false-negative (same effective type, different Type instances
   * but we do not care, correct check (by comparing info() result
   * is more expensive than the dummy conversion that will happen.
  */
    if (type == targetType)
    {
      return std::make_pair(*this, false);
    }

    GenericValuePtr result;
    Type::Kind skind = type->kind();
    Type::Kind dkind = targetType->kind();
    if (skind == dkind)
    {
      switch(skind)
      {
        case Type::Float:
          result.type = targetType;
          result.value = targetType->initializeStorage();
          static_cast<TypeFloat*>(targetType)->set(&result.value,
                                                   static_cast<TypeFloat*>(type)->get(value));
          return std::make_pair(result, true);
        case Type::Int:
          result.type = targetType;
          result.value = targetType->initializeStorage();
          static_cast<TypeInt*>(targetType)->set(&result.value,
                                                 static_cast<TypeInt*>(type)->get(value));
          return std::make_pair(result, true);
        case Type::String:
          {
            if (targetType->info() == type->info())
              return std::make_pair(*this, false);
            result.type = targetType;
            result.value = targetType->initializeStorage();
            std::pair<char*, size_t> v = static_cast<TypeString*>(type)->get(value);
            static_cast<TypeString*>(targetType)->set(&result.value,
                                                      v.first, v.second);
            return std::make_pair(result, true);
          }
        case Type::List:
        {
          result.type = targetType;
          GenericListPtr lsrc = asList();
          TypeList* targetListType = static_cast<TypeList*>(targetType);
          Type* srcElemType = lsrc.elementType();
          void* storage = targetType->initializeStorage();
          Type* dstElemType = targetListType->elementType();
          bool needConvert = (srcElemType->info() != dstElemType->info());
          GenericListPtr lresult;
          lresult.type = targetListType;
          lresult.value = storage;
          GenericListIteratorPtr i = lsrc.begin();
          GenericListIteratorPtr iend = lsrc.end();
          for (; i!= iend; ++i)
          {
            GenericValuePtr val = *i;
            if (!needConvert)
              lresult.pushBack(val);
            else
            {
              std::pair<GenericValuePtr,bool> c = val.convert(dstElemType);
              lresult.pushBack(c.first);
              if (c.second)
                c.first.destroy();
            }
          }
          return std::make_pair(lresult, true);
        }
          break;
        case Type::Map:
        {
          result.type = targetType;
          GenericMapPtr msrc = asMap();


          TypeMap* targetMapType = static_cast<TypeMap*>(targetType);
          TypeMap* srcMapType = static_cast<TypeMap*>(type);

          Type* srcKeyType = srcMapType->keyType();
          Type* srcElementType = srcMapType->elementType();


          GenericMapPtr mresult;
          mresult.type = targetType;
          mresult.value = targetMapType->initializeStorage();
          Type* targetKeyType = targetMapType->keyType();
          Type* targetElementType = targetMapType->elementType();

          bool sameKey = srcKeyType->info() == targetKeyType->info();
          bool sameElem = srcElementType->info() == targetElementType->info();

          GenericMapIteratorPtr i = msrc.begin();
          GenericMapIteratorPtr iend = msrc.end();
          for (; i != iend; ++i)
          {
            std::pair<GenericValuePtr, GenericValuePtr> kv = *i;
            std::pair<GenericValuePtr, bool> ck, cv;
            if (!sameKey)
            {
              ck = kv.first.convert(targetKeyType);
              if (!ck.first.type)
                return std::make_pair(GenericValuePtr(), false);
            }
            if (!sameElem)
            {
              cv = kv.second.convert(targetElementType);
              if (!cv.first.type)
                return std::make_pair(GenericValuePtr(), false);
            }
            mresult.insert(sameKey?kv.first:ck.first, sameElem?kv.second:cv.first);
            if (!sameKey && ck.second)
              ck.first.destroy();
            if (!sameElem && cv.second)
              cv.first.destroy();
          }
          return std::make_pair(mresult, true);
        }
          break;
        case Type::Pointer:
        {
          Type* srcPointedType = static_cast<TypePointer*>(type)->pointedType();
          Type* dstPointedType = static_cast<TypePointer*>(targetType)->pointedType();
          // We only try to handle conversion for pointer to objects
          if (srcPointedType->kind() != Type::Object || dstPointedType->kind() != Type::Object)
          {
            // However, we need the full check for exact match here
            if (type->info() == targetType->info())
              return std::make_pair(*this, false);
            else
              return std::make_pair(GenericValuePtr(), false);
          }
          GenericValuePtr pointedSrc = static_cast<TypePointer*>(type)->dereference(value);
          std::pair<GenericValuePtr, bool> pointedDstPair = pointedSrc.convert(dstPointedType);
          if (!pointedDstPair.first.type)
            return std::make_pair(GenericValuePtr(), false);
          if (pointedDstPair.second)
            qiLogError("qi.meta") << "assertion error, allocated converted reference";
          // We must re-reference
          GenericValuePtr pointedDst = pointedDstPair.first;
          void* ptr = pointedDst.type->ptrFromStorage(&pointedDst.value);
          result.type = targetType;
          result.value = targetType->initializeStorage(&ptr);
          return std::make_pair(result, false);
        }
          break;
        case Type::Tuple:
        {
          TypeTuple* tsrc = static_cast<TypeTuple*>(type);
          TypeTuple* tdst = static_cast<TypeTuple*>(targetType);
          std::vector<void*> sourceData = tsrc->get(value);
          std::vector<Type*> srcTypes = tsrc->memberTypes();
          std::vector<Type*> dstTypes = tdst->memberTypes();
          if (dstTypes.size() != sourceData.size())
          {
            qiLogWarning("qi.meta") << "Conversion failure: tuple size mismatch";
            return std::make_pair(GenericValuePtr(), false);
          }

          std::vector<void*> targetData;
          std::vector<bool> mustDestroy;
          for (unsigned i=0; i<dstTypes.size(); ++i)
          {
            std::pair<GenericValuePtr, bool> conv = GenericValuePtr(srcTypes[i], sourceData[i]).convert(dstTypes[i]);
            if (!conv.first.type)
            {
              qiLogWarning("qi.meta") << "Conversion failure in tuple member between "
                                      << srcTypes[i]->infoString() << " and " << dstTypes[i]->infoString();
              return std::make_pair(GenericValuePtr(), false);
            }
            targetData.push_back(conv.first.value);
            mustDestroy.push_back(conv.second);
          }
          void* dst = tdst->initializeStorage();
          tdst->set(&dst, targetData);
          for (unsigned i=0; i<mustDestroy.size(); ++i)
          {
            if (mustDestroy[i])
              dstTypes[i]->destroy(targetData[i]);
          }
          result.type = targetType;
          result.value = dst;
          return std::make_pair(result, true);
        }
        case Type::Dynamic: {
          result.type  = targetType;
          result.value = targetType->initializeStorage();
          static_cast<TypeDynamic*>(targetType)->set(&result.value, *this);
          return std::make_pair(result, true);
        }
        case Type::Raw: {
          result.type = targetType;
          result.value = targetType->initializeStorage();
          qi::Buffer buf = static_cast<TypeRaw*>(type)->get(value);
          static_cast<TypeRaw*>(targetType)->set(&result.value, buf);
          return std::make_pair(result, true);
        }
        default:
          break;
      } // switch
    } // skind == dkind
    if (skind == Type::Float && dkind == Type::Int)
    {
      result.type = targetType;
      result.value = targetType->initializeStorage();
      static_cast<TypeInt*>(targetType)->set(&result.value,
        static_cast<qi::int64_t>(
          static_cast<TypeFloat*>(type)->get(value)));
      return std::make_pair(result, true);
    }
    else if (skind == Type::Int && dkind == Type::Float)
    {
      result.type = targetType;
      result.value = targetType->initializeStorage();
      static_cast<TypeFloat*>(targetType)->set(&result.value,
        static_cast<double>(
          static_cast<TypeInt*>(type)->get(value)));
      return std::make_pair(result, true);
    }
    else if (skind == Type::String && dkind == Type::Raw)
    {
      qi::Buffer buf;
      std::pair<char*, size_t> data = static_cast<TypeString*>(type)->get(value);
      memcpy(buf.reserve(data.second), data.first, data.second);
      result.type = targetType;
      result.value = targetType->initializeStorage();
      static_cast<TypeRaw*>(result.type)->set(&result.value, buf);
      return std::make_pair(result, true);
    }
    else if (skind == Type::Raw && dkind == Type::String)
    {
      qiLogWarning("qi.meta") << "Conversion attempt from raw to string";
      return std::make_pair(GenericValuePtr(), false);
    }
    if (targetType->kind() == Type::Dynamic)
    {
      result.type = targetType;
      result.value = targetType->initializeStorage();
      static_cast<TypeDynamic*>(targetType)->set(&result.value, *this);
      return std::make_pair(result, true);
    }
    if (type->kind() == Type::Dynamic)
    {
      std::pair<GenericValuePtr, bool> gv = ((TypeDynamic*)type)->get(value);
      std::pair<GenericValuePtr, bool> result = gv.first.convert(targetType);
      if (gv.second && !result.second)
      { // Result is using memory from gv, but maybe only part of it
        result.first = result.first.clone();
        result.second = true;
      }
      if (gv.second)
        gv.first.destroy();
      return result;
    }
    static Type* genericValueType = typeOf<GenericValuePtr>();
    static Type* genericObjectType = typeOf<GenericObject>();

    if (type->info() == genericValueType->info())
    { // Source is metavalue: special case
      GenericValuePtr* metaval = (GenericValuePtr*)value;
      return metaval->convert(targetType);
    }
    if (type->info() == genericObjectType->info())
    {
      GenericObject* obj = (GenericObject*)value;
      GenericValuePtr v;
      v.type = obj->type;
      v.value = obj->value;
      return v.convert(targetType);
    }
    if (skind == Type::Object)
    {
      // Try inheritance
      ObjectType* osrc = static_cast<ObjectType*>(type);
      qiLogDebug("qi.meta") << "inheritance check "
                            << osrc <<" " << (osrc?osrc->inherits(targetType):false);
      int inheritOffset = 0;
      if (osrc && (inheritOffset =  osrc->inherits(targetType)) != -1)
      {
        // We return a Value that point to the same data as this.
        result.type = targetType;
        result.value = (void*)((long)value + inheritOffset);
        return std::make_pair(result, false);
      }
    }
    if (type->info() == targetType->info())
    {
      return std::make_pair(*this, false);
    }

    return std::make_pair(GenericValuePtr(), false);
  }

  GenericValuePtr GenericValuePtr::convertCopy(Type* targetType) const
  {
    std::pair<GenericValuePtr, bool> res = convert(targetType);
    if (res.second)
      return res.first;
    else
      return res.first.clone();
  }

  bool operator< (const qi::GenericValuePtr& a, const qi::GenericValuePtr& b)
  {
    return a.value < b.value;
  }

}

QI_TYPE_REGISTER(qi::GenericValuePtr);
