/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#include <boost/lexical_cast.hpp>

#include <qitype/genericvalue.hpp>
#include <qitype/genericobject.hpp>

qiLogCategory("qitype.genericvalue");

namespace qi
{

  std::pair<GenericValuePtr, bool> GenericValuePtr::convert(Type* targetType) const
  {
    // qiLogDebug() << "convert " << type->infoString() << ' ' << targetType->infoString();
    /* Can have false-negative (same effective type, different Type instances
     * but we do not care, correct check (by comparing info() result
     * is more expensive than the dummy conversion that will happen.
     */
    if (type == targetType)
    {
      return std::make_pair(*this, false);
    }

    if (!targetType || !type) {
      qiLogWarning() << "Conversion error: can't convert to/from a null type.";
      return std::make_pair(GenericValuePtr(), false);
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
          i.destroy();
          iend.destroy();
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
          i.destroy();
          iend.destroy();
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
            qiLogError() << "assertion error, allocated converted reference";
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
            qiLogWarning() << "Conversion failure: tuple size mismatch";
            return std::make_pair(GenericValuePtr(), false);
          }

          std::vector<void*> targetData;
          std::vector<bool> mustDestroy;
          for (unsigned i=0; i<dstTypes.size(); ++i)
          {
            std::pair<GenericValuePtr, bool> conv = GenericValuePtr(srcTypes[i], sourceData[i]).convert(dstTypes[i]);
            if (!conv.first.type)
            {
              qiLogWarning() << "Conversion failure in tuple member between "
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
      qiLogWarning() << "Conversion attempt from raw to string";
      return std::make_pair(GenericValuePtr(), false);
    }
    if (targetType->kind() == Type::Dynamic)
    {
      result.type = targetType;
      result.value = targetType->initializeStorage();
      static_cast<TypeDynamic*>(targetType)->set(&result.value, *this);
      return std::make_pair(result, true);
    }
    if (type->info() == typeOf<ObjectPtr>()->info()
      && targetType->kind() == Type::Pointer
    && static_cast<TypePointer*>(targetType)->pointedType()->kind() == Type::Object)
    { // Attempt specialized proxy conversion
      detail::ProxyGeneratorMap& map = detail::proxyGeneratorMap();
      detail::ProxyGeneratorMap::iterator it = map.find(
        static_cast<TypePointer*>(targetType)->pointedType()->info());
      if (it != map.end())
      {
        GenericValuePtr res = (it->second)(*(ObjectPtr*)value);
        return std::make_pair(res, true);
      }
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

    if (skind == Type::Object && dkind == Type::Pointer)
    {
      std::pair<GenericValuePtr, bool> gv = convert(
        static_cast<TypePointer*>(targetType)->pointedType());
      if (!gv.first.type)
        return gv;
      // Re-pointerise it
      void* ptr = gv.first.type->ptrFromStorage(&gv.first.value);
      GenericValuePtr result;
      result.type = targetType;
      result.value = targetType->initializeStorage(&ptr);
      return std::make_pair(result, false);
    }
    if (skind == Type::Object)
    {
      // Try inheritance
      ObjectType* osrc = static_cast<ObjectType*>(type);
      qiLogDebug() << "inheritance check "
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

  bool operator< (const GenericValuePtr& a, const GenericValuePtr& b)
  {
    qiLogDebug() << "Compare " << a.type << ' ' << b.type;
    #define GET(v, t) static_cast<Type ## t *>(v.type)->get(v.value)
    if (!a.type)
      return b.type;
    if (!b.type)
      return false;
    /* < operator for char* does not do what we want, so force
    * usage of get() below for string types.
    */
    if ((a.type == b.type || a.type->info() == b.type->info())
      && a.type->kind() != Type::String)
    {
      qiLogDebug() << "Compare sametype " << a.type->infoString();
      return a.type->less(a.value, b.value);
    }
    // Comparing values of different types
    Type::Kind ka = a.type->kind();
    Type::Kind kb = b.type->kind();
    qiLogDebug() << "Compare " << ka << ' ' << kb;
    if (ka != kb)
    {
      if (ka == Type::Int && kb == Type::Float)
        return GET(a, Int) < GET(b, Float);
      else if (ka == Type::Float && kb == Type::Int)
        return GET(a, Float) < GET(b, Int);
      else
        return ka < kb; // Safer than comparing pointers
    }
    else switch(ka)
    {
    case Type::Void:
      return false;
    case Type::Int:
      return GET(a, Int) < GET(b, Int);
    case Type::Float:
      return GET(a, Float) < GET(b, Float);
    case Type::String:
      {
        std::pair<char*, size_t> ca, cb;
        ca = GET(a, String);
        cb = GET(b, String);
        bool res = ca.second == cb.second?
        (memcmp(ca.first, cb.first, ca.second) < 0) : (ca.second < cb.second);
        qiLogDebug() << "Compare " << ca.first << ' ' << cb.first << ' ' << res;
        return res;
      }
    case Type::List:
      {
        TypeList* ta = static_cast<TypeList*>(a.type);
        TypeList* tb = static_cast<TypeList*>(b.type);
        size_t la = ta->size(a.value);
        size_t lb = tb->size(b.value);
        if (la != lb)
          return la < lb;
        GenericListIteratorPtr ita   = ta->begin(a.value);
        GenericListIteratorPtr enda = ta->end(a.value);
        GenericListIteratorPtr itb   = tb->begin(b.value);
        GenericListIteratorPtr endb = tb->end(b.value);
        bool result = false;
        while (! (ita == enda))
        {
          assert (! (itb == endb));
          GenericValuePtr ea = *ita;
          GenericValuePtr eb = *itb;
          if (ea < eb)
          {
            result = true;
            goto cleanuplist;
          }
          else if (eb < ea)
          {
            result = false;
            goto cleanuplist;
          }
          ++ita;
          ++itb;
        }
      cleanuplist:
        ita.destroy();
        itb.destroy();
        enda.destroy();
        endb.destroy();
        return result;
      }
    case Type::Map:
      { // An astute observer will notice a lot of similarities
        // with list code above
        TypeMap* ta = static_cast<TypeMap*>(a.type);
        TypeMap* tb = static_cast<TypeMap*>(b.type);
        size_t la = ta->size(a.value);
        size_t lb = tb->size(b.value);
        if (la != lb)
          return la < lb;
        GenericMapIteratorPtr ita   = ta->begin(a.value);
        GenericMapIteratorPtr enda = ta->end(a.value);
        GenericMapIteratorPtr itb   = tb->begin(b.value);
        GenericMapIteratorPtr endb = tb->end(b.value);
        bool result = false;
        while (! (ita == enda))
        {
          assert (! (itb == endb));
          std::pair<GenericValuePtr, GenericValuePtr> ea = *ita;
          std::pair<GenericValuePtr, GenericValuePtr> eb = *itb;
          if (ea < eb)
          {
            result = true;
            goto cleanupmap;
          }
          else if (eb < ea)
          {
            result = false;
            goto cleanupmap;
          }
          ++ita;
          ++itb;
        }
      cleanupmap:
        ita.destroy();
        itb.destroy();
        enda.destroy();
        endb.destroy();
        return result;
      }
    case Type::Object:
    case Type::Pointer:
    case Type::Tuple:
    case Type::Dynamic:
    case Type::Raw:
    case Type::Unknown:
      return a.value < b.value;
    }
    #undef GET
    return a.value < b.value;
  }
  bool operator< (const GenericValue& a, const GenericValue& b)
  {
    return (const GenericValuePtr&)a < (const GenericValuePtr&)b;
  }

  bool operator==(const GenericValuePtr& a, const GenericValuePtr& b)
  {
    return ! (a < b) && !(b<a);
  }

  void GenericTuplePtr::set(const std::vector<GenericValuePtr>& data)
  {
    TypeTuple* typeTuple = static_cast<TypeTuple*>(type);
    std::vector<Type*> mTypes = memberTypes();
    if (data.size() != mTypes.size())
      throw std::runtime_error("Tuple size mismatch, expected "
        + boost::lexical_cast<std::string>(mTypes.size())
        + " got "
        + boost::lexical_cast<std::string>(data.size())
      );
    // FIXME factor this with GenericFunctionParameters::copy
    std::vector<void*> convData;
    convData.reserve(data.size());
    // Store indexes that where copied by converter and that needs a destroy
    std::vector<int> allocatedIndexes;
    for (unsigned i=0; i<data.size(); ++i)
    {
      std::pair<GenericValuePtr, bool> r = data[i].convert(mTypes[i]);
      if (!r.first.type)
        throw std::runtime_error(std::string("Conversion failure from ")
          + data[i].type->infoString()
          + " to "
          + mTypes[i]->infoString());
      convData.push_back(r.first.value);
      if (r.second)
        allocatedIndexes.push_back(i);
    }
    typeTuple->set(&value, convData);
    for (unsigned i=0; i<allocatedIndexes.size(); ++i)
      mTypes[allocatedIndexes[i]]->destroy(convData[allocatedIndexes[i]]);
  }

  GenericValue GenericValuePtr::toTuple(bool homogeneous) const
  {
    if (kind() == Type::Tuple)
      return GenericValue(*this);
    else if (kind() != Type::List)
      throw std::runtime_error("Expected Tuple or List kind");
    // convert list to tuple

    TypeList* t = static_cast<TypeList*>(type);
    Type* te = t->elementType();
    TypeDynamic* td = 0;
    if (te->kind() == Type::Dynamic)
      td = static_cast<TypeDynamic*>(te);
    if (!homogeneous && !td)
      throw std::runtime_error("Element type is not dynamic");
    std::vector<GenericValuePtr> elems;
    GenericListIteratorPtr it = t->begin(value);
    GenericListIteratorPtr iend = t->end(value);
    std::vector<GenericValuePtr> cleanup;
    while (it != iend)
    {
      GenericValuePtr e = *it;
      if (homogeneous)
        elems.push_back(e);
      else
      {
        std::pair<GenericValuePtr, bool> de = td->get(e.value);
        elems.push_back(de.first);
        if (de.second)
          cleanup.push_back(de.first);
      }
      ++it;
    }
    it.destroy();
    iend.destroy();
    GenericValue res(makeGenericTuple(elems));
    for (unsigned i=0; i<cleanup.size(); ++i)
      cleanup[i].destroy();
    return res;
  }

  ObjectPtr GenericValuePtr::toObject() const
  {
    return to<ObjectPtr>();
  }

  GenericValuePtr GenericValuePtr::_element(const GenericValuePtr& key, bool throwOnFailure)
  {
    if (kind() == Type::List)
    {
      TypeList* t = static_cast<TypeList*>(type);
      int ikey = key.toInt();
      if (ikey < 0 || static_cast<size_t>(ikey) >= t->size(value))
      {
        if (throwOnFailure)
          throw std::runtime_error("Index out of range");
        else
          return GenericValuePtr();
      }
      return GenericValuePtr(t->elementType(), t->element(value, ikey));
    }
    else if (kind() == Type::Map)
    {
      TypeMap* t = static_cast<TypeMap*>(type);
      std::pair<GenericValuePtr, bool> c = key.convert(t->keyType());
      if (!c.first.type)
        throw std::runtime_error("Incompatible key type");
      // HACK: should be two separate booleans
      bool autoInsert = throwOnFailure;
      GenericValuePtr result
        = t->element(&value, c.first.value, autoInsert);
      if (c.second)
        c.first.destroy();
      return result;
    }
    else if (kind() == Type::Tuple)
    {
      TypeTuple* t = static_cast<TypeTuple*>(type);
      int ikey = key.toInt();
      std::vector<Type*> types = t->memberTypes();
      if (ikey < 0 || static_cast<size_t>(ikey) >= types.size())
      {
        if (throwOnFailure)
          throw std::runtime_error("Index out of range");
        else
          return GenericValuePtr();
      }
      return GenericValuePtr(types[ikey], t->get(value, ikey));
    }
    else
      throw std::runtime_error("Expected List, Map or Tuple kind");
  }

  void GenericValuePtr::_append(const GenericValuePtr& elem)
  {
    if (kind() != Type::List)
      throw std::runtime_error("Expected a list");
    TypeList* t = static_cast<TypeList*>(type);
    std::pair<GenericValuePtr, bool> c = elem.convert(t->elementType());
    t->pushBack(&value, c.first.value);
    if (c.second)
      c.first.destroy();
  }

  void GenericValuePtr::_insert(const GenericValuePtr& key, const GenericValuePtr& val)
  {
    if (kind() != Type::Map)
      throw std::runtime_error("Expected a map");
    std::pair<GenericValuePtr, bool> ck(key, false);
    std::pair<GenericValuePtr, bool> cv(val, false);
    TypeMap* t = static_cast<TypeMap*>(type);
    if (key.type != t->keyType())
      ck = key.convert(t->keyType());
    if (val.type != t->elementType())
      cv = val.convert(t->elementType());
    t->insert(&value, ck.first.value, cv.first.value);
    if (ck.second)
      ck.first.destroy();
    if (cv.second)
      cv.first.destroy();
  }

  void GenericValuePtr::update(const GenericValuePtr& val)
  {
    switch(kind())
    {
    case Type::Int:
      setInt(val.toInt());
      break;
    case Type::Float:
      setDouble(val.toDouble());
      break;
    case Type::String:
      setString(val.toString());
      break;
    default:
      throw std::runtime_error("Update not implemented for this type.");
    }
  }
}

