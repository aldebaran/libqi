/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/range/algorithm/transform.hpp>

#include <qi/type/detail/anyreference.hpp>
#include <qi/anyobject.hpp>

#include <ka/errorhandling.hpp>

#if defined(_MSC_VER) && _MSC_VER <= 1500
// vs2008 32 bits does not have std::abs() on int64
namespace std
{
  qi::int64_t abs(qi::int64_t x)
  {
    if (x < 0)
      return -x;
    else
      return x;
  }
}
#endif

qiLogCategory("qitype.genericvalue");

namespace qi
{

namespace
{
  static void dropIt(GenericObject* ptr, const AnyValue& /*v*/)
  {
    qiLogDebug() << "dropIt";
    delete ptr;
  }

  struct DefaultUniqueAnyRef
  {
    template<typename... Args>
    detail::UniqueAnyReference operator()(Args&&...) const
    {
      return detail::UniqueAnyReference{};
    }
  };
}

namespace detail
{

  UniqueAnyReference AnyReferenceBase::convert(DynamicTypeInterface* targetType) const
  {
    if (!targetType)
      return {};

    // same-type check (not done before, useful mainly for AnyObject)
    if (targetType->info() == type()->info())
      return UniqueAnyReference{ *this, DeferOwnership{} };

    return ka::invoke_catch(DefaultUniqueAnyRef{},
                            [&] {
                              UniqueAnyReference result { AnyReference{ targetType } };
                              static_cast<DynamicTypeInterface*>(targetType)
                                  ->set(&result->_value, AnyReference(*this));
                              return result;
                            });
  }

  UniqueAnyReference AnyReferenceBase::convert(PointerTypeInterface* targetType) const
  {
    if (!targetType)
      return {};

    switch (_type->kind())
    {
    case TypeKind_Pointer:
    {
      TypeInterface* srcPointedType = static_cast<PointerTypeInterface*>(_type)->pointedType();
      TypeInterface* dstPointedType = static_cast<PointerTypeInterface*>(targetType)->pointedType();
      qiLogDebug() << "Pointees are " << srcPointedType->infoString() << " " << dstPointedType->infoString();
      // We only try to handle conversion for pointer to objects
      if (srcPointedType->kind() != TypeKind_Object || dstPointedType->kind() != TypeKind_Object)
      {
        // However, we need the full check for exact match here
        if (_type->info() == targetType->info())
          return UniqueAnyReference{ *this, DeferOwnership{} };
        else
        {
          qiLogDebug() << "Conversion between non-object pointers not supported";
          return {};
        }
      }
      AnyReference pointedSrc = static_cast<PointerTypeInterface*>(_type)->dereference(_value);
      // try object conversion (inheritance)
      auto pointedDstPair = pointedSrc.convert(dstPointedType);
      if (!pointedDstPair->_type)
      {
        qiLogDebug() << "Attempting object->proxy conversion";
        // try object->proxy conversion by simply rewrapping this
        detail::ManagedObjectPtr o(
              new GenericObject(
                static_cast<ObjectTypeInterface*>(pointedSrc._type),
                pointedSrc._value, pointedSrc.to<AnyObject>().uid()),
              boost::bind(dropIt, _1, qi::AnyValue(*this)));
        return AnyReference::from(o).convert((TypeInterface*)targetType);
      }
      if (pointedDstPair.ownsReference())
        qiLogError() << "assertion error, allocated converted reference";
      return ka::invoke_catch(DefaultUniqueAnyRef{}, [&] {
        // We must re-reference
        AnyReference pointedDst = *pointedDstPair;
        void* ptr = pointedDst._type->ptrFromStorage(&pointedDst._value);

        UniqueAnyReference result{ AnyReference{ targetType } };
        targetType->setPointee(&result->_value, ptr);
        return UniqueAnyReference{ result.release(), DeferOwnership{}};
      });
    }
    case TypeKind_Object:
    {
      auto gv = convert(static_cast<PointerTypeInterface*>(targetType)->pointedType());
      if (!gv->_type)
        return {};
      return ka::invoke_catch(DefaultUniqueAnyRef{}, [&] {
        // Re-pointerise it
        void* ptr = gv->_type->ptrFromStorage(&gv->_value);
        UniqueAnyReference result{ AnyReference{ targetType } };
        targetType->setPointee(&result->_value, ptr);
        return UniqueAnyReference{ result.release(), DeferOwnership{}};
      });
    }
    default:
      break;
    }
    return {};
  }

  UniqueAnyReference AnyReferenceBase::convert(ListTypeInterface* targetType) const
  {
    if (!targetType)
      return {};

    switch (_type->kind())
    {
    case TypeKind_VarArgs:
    case TypeKind_List:
    {
      return ka::invoke_catch(DefaultUniqueAnyRef{}, [&]() -> UniqueAnyReference {
        ListTypeInterface* targetListType = static_cast<ListTypeInterface*>(targetType);
        ListTypeInterface* sourceListType = static_cast<ListTypeInterface*>(_type);

        TypeInterface* srcElemType = sourceListType->elementType();
        TypeInterface* dstElemType = targetListType->elementType();
        bool needConvert = (srcElemType->info() != dstElemType->info());
        UniqueAnyReference result{ AnyReference{ targetListType } };
        for (auto val : *this)
        {
          if (!needConvert)
            result->append(val);
          else
          {
            auto c = val.convert(dstElemType);
            if (!c->_type)
            {
              qiLogDebug() << "List element conversion failure from " << val._type->infoString()
                           << " to " << dstElemType->infoString();
              return {};
            }
            result->append(*c);
          }
        }
        return result;
      });
    }
    default:
      break;
    }
    return {};
  }

  UniqueAnyReference AnyReferenceBase::convert(StringTypeInterface* targetType) const
  {
    if (!targetType)
      return {};

    switch (_type->kind())
    {
    case TypeKind_String:
    {
      if (targetType->info() == _type->info())
        return UniqueAnyReference{ *this, DeferOwnership{} };
      return ka::invoke_catch(DefaultUniqueAnyRef{}, [&] {
        UniqueAnyReference result{ AnyReference{ targetType } };
        StringTypeInterface::ManagedRawString v =
            static_cast<StringTypeInterface*>(_type)->get(_value);
        targetType->set(&result->_value, v.first.first, v.first.second);
        if (v.second)
          v.second(v.first);
        return result;
      });
    }
    case TypeKind_Raw:
    {
      qiLogWarning() << "Conversion attempt from raw to string";
      return {};
    }
    default:
      break;
    }
    return {};
  }

  UniqueAnyReference AnyReferenceBase::convert(RawTypeInterface* targetType) const
  {
    if (!targetType)
      return {};

    switch (_type->kind())
    {
    case TypeKind_Raw:
    {
      if (targetType->info() == _type->info())
        return UniqueAnyReference{ *this, DeferOwnership{} };
      return ka::invoke_catch(DefaultUniqueAnyRef{}, [&] {
        UniqueAnyReference result{ AnyReference { targetType } };
        std::pair<char*, size_t> v = static_cast<RawTypeInterface*>(_type)->get(_value);
        static_cast<RawTypeInterface*>(result->_type)->set(&result->_value, v.first, v.second);
        return result;
      });
    }
    case TypeKind_String:
    {
      return ka::invoke_catch(DefaultUniqueAnyRef{}, [&] {
        StringTypeInterface::ManagedRawString data =
            static_cast<StringTypeInterface*>(_type)->get(_value);
        UniqueAnyReference result{ AnyReference{ targetType } };
        static_cast<RawTypeInterface*>(result->_type)
            ->set(&result->_value, data.first.first, data.first.second);
        if (data.second)
          data.second(data.first);
        return result;
      });
    }
    default:
      break;
    }
    return {};
  }

  UniqueAnyReference AnyReferenceBase::convert(FloatTypeInterface* targetType) const
  {
    if (!targetType)
      return {};

    switch (_type->kind())
    {
    case TypeKind_Float:
    {
      return ka::invoke_catch(DefaultUniqueAnyRef{}, [&] {
        UniqueAnyReference result{ AnyReference{ targetType } };
        static_cast<FloatTypeInterface*>(targetType)
            ->set(&result->_value, static_cast<FloatTypeInterface*>(_type)->get(_value));
        return result;
      });
    }
    case TypeKind_Int:
    {
      return ka::invoke_catch(DefaultUniqueAnyRef{}, [&] {
        UniqueAnyReference result{ AnyReference{ targetType } };
        int64_t v = static_cast<IntTypeInterface*>(_type)->get(_value);
        if (static_cast<IntTypeInterface*>(_type)->isSigned())
          result->setInt(v);
        else
          result->setUInt((uint64_t)v);
        return result;
      });
    }
    default:
      break;
    }
    return {};
  }

  UniqueAnyReference AnyReferenceBase::convert(IntTypeInterface* targetType) const
  {
    if (!targetType)
      return {};

    switch (_type->kind())
    {
    case TypeKind_Int:
    {
      return ka::invoke_catch(DefaultUniqueAnyRef{}, [&] {
        IntTypeInterface* tsrc = static_cast<IntTypeInterface*>(_type);
        IntTypeInterface* tdst = static_cast<IntTypeInterface*>(targetType);
        int64_t v = tsrc->get(_value);
        /* Bounce to GVP to perform overflow checks
        */
        UniqueAnyReference result{ AnyReference{ tdst } };
        if (tsrc->isSigned())
          result->setInt(v);
        else
          result->setUInt((uint64_t)v);
        tdst->set(&result->_value, v);
        return result;
      });
    }
    case TypeKind_Float:
    {
      return ka::invoke_catch(DefaultUniqueAnyRef{}, [&] {
        double v = static_cast<FloatTypeInterface*>(_type)->get(_value);
        IntTypeInterface* tdst = static_cast<IntTypeInterface*>(targetType);
        UniqueAnyReference result{ AnyReference{ tdst } };
        // bounce to setDouble for overflow check
        result->setDouble(v);
        return result;
      });
    }
    default:
      break;
    }
    return {};
  }

  UniqueAnyReference AnyReferenceBase::convert(OptionalTypeInterface* targetType) const
  {
    if (!targetType)
      return {};

    return ka::invoke_catch(DefaultUniqueAnyRef{}, [&] {
      const auto optValType = targetType->valueType();
      UniqueAnyReference result{ AnyReference{ targetType } };
      switch (_type->kind())
      {
      case TypeKind_Void:
        // do nothing, converting void to optional means creating an unset
        // optional
        break;
      case TypeKind_Optional:
      {
        const auto thisType = static_cast<OptionalTypeInterface*>(_type);
        const auto thisValueType = thisType->valueType();

        // We do not allow the conversion of Opt<T> to Opt<U> if T is not convertible to U, even if
        // the instance of Opt<T> is empty. An exception to this is if T is Dynamic because it might
        // work in some cases (where the dynamic value is convertible to U).
        if (thisValueType->kind() != TypeKind_Dynamic &&
            thisValueType->signature().isConvertibleTo(optValType->signature()) == 0.f)
        {
          throwConversionFailure(_type, optValType, "optional value type is not convertible to"
                                 " target optional value type.");
        }
        result->setOptional(asOptional());
        break;
      }
      // In all other cases, try setting this value as the value of the
      // optional
      default:
        // Recurse the conversion to the value type of the optional
        auto converted = convert(optValType);
        if (!converted->isValid())
          throwConversionFailure(_type, optValType, "");
        targetType->set(&result->_value, converted->_value);
        break;
      }
      return result;
    });
  }

  static std::string fieldList(const std::map<std::string, qi::AnyReference>& map)
  {
    std::string ret;
    for (const auto& item : map)
    {
      if (!ret.empty())
        ret += ", ";
      ret += item.first;
    }
    return ret;
  }

  static UniqueAnyReference structConverter(const AnyReferenceBase* src, StructTypeInterface* tdst)
  {
    StructTypeInterface* tsrc = static_cast<StructTypeInterface*>(src->type());

    const std::vector<std::string> srcNames = tsrc->elementsName();
    const std::vector<std::string> dstNames = tdst->elementsName();
    const std::vector<TypeInterface*> srcTypes = tsrc->memberTypes();
    const std::vector<TypeInterface*> dstTypes = tdst->memberTypes();
    if (srcTypes.size() != srcNames.size() || dstTypes.size() != dstNames.size())
    {
      qiLogVerbose() << "Cannot convert between not fully named mismatching tuples " << tsrc->infoString() << " and "
                     << tdst->infoString();
      return {};
    }
    // Compute mapping between src and dst fields based on names
    std::vector<int> fieldMap; // fieldMap[i] = index of src's field i in dst (-1 for not present)
    std::map<std::string, qi::AnyReference> fieldDrop; // unused src fields
    for (unsigned i = 0; i < srcNames.size(); ++i)
    {
      std::vector<std::string>::const_iterator it = std::find(dstNames.begin(), dstNames.end(), srcNames[i]);
      if (it != dstNames.end())
        fieldMap.push_back(static_cast<int>(it - dstNames.begin()));
      else
      {
        fieldDrop[srcNames[i]] = AnyReference(srcTypes[i], tsrc->get(src->rawValue(), i));
        fieldMap.push_back(-1);
      }
    }
    std::vector<std::tuple<std::string, TypeInterface*>> fieldMissing; // unfilled dst fields
    for (unsigned i = 0; i < dstNames.size(); ++i)
    {
      std::vector<int>::iterator it = std::find(fieldMap.begin(), fieldMap.end(), i);
      if (it == fieldMap.end())
        fieldMissing.push_back(std::make_tuple(dstNames[i], dstTypes[i]));
    }
    auto vecOfTuplesToStrings = [](const std::vector<std::tuple<std::string, TypeInterface*>>& vec) {
      std::string out;
      for (const auto& t : vec)
      {
        if (!out.empty())
          out += ", ";
        out += std::get<0>(t);
      }
      return out;
    };
    qiLogDebug() << "Field mapping:"
                 << " drop=" << fieldList(fieldDrop)
                 << " missing=" << vecOfTuplesToStrings(fieldMissing);

    // convert what we can (missing field check might need the data)
    std::vector<UniqueAnyReference> uniqueConvertedFields;
    uniqueConvertedFields.reserve(dstTypes.size());
    std::vector<void*> targetData(dstTypes.size(), nullptr);

    const std::vector<void*> sourceData = tsrc->get(src->rawValue());

    for (unsigned i = 0; i < srcTypes.size(); ++i)
    {
      int targetIndex = fieldMap[i];
      if (targetIndex == -1)
        continue; // dropped field, do not convert
      auto conv = AnyReference(srcTypes[i], sourceData[i]).convert(dstTypes[targetIndex]);
      if (!conv->type())
      {
        qiLogVerbose() << "Conversion failure in tuple member " << srcNames[i] << " between "
                       << srcTypes[i]->infoString() << " and " << dstTypes[targetIndex]->infoString();
        return {};
      }
      uniqueConvertedFields.emplace_back(std::move(conv));
      targetData[targetIndex] = uniqueConvertedFields.back()->rawValue();
    }

    // Then ask source and target if it's okay to drop fields and fill missing fields
    std::map<std::string, AnyValue> fields; // used only in if below but must survive longuer
    if (!fieldMissing.empty() || !fieldDrop.empty())
    {
      // Unfortunately we cannot instanciate target type, because of the
      // missing fields (in case struct is in constructor mode), so present
      // available fields as a map

      // petty optimization until AnyValue has proper refcount behavior
      // we transfer ownership to the map, so that fillMissing can replace
      // existing values
      // Preallocates all elements so that stuff dont move
      for (unsigned i = 0; i < dstNames.size(); ++i)
        fields[dstNames[i]] = AnyValue();
      // Fill elements we have, transfering ownership
      for (unsigned i = 0; i < dstNames.size(); ++i)
      {
        if (std::find(fieldMap.begin(), fieldMap.end(), i) != fieldMap.end())
        {
          auto& uniqueConvertedField = uniqueConvertedFields[i];
          fields[dstNames[i]].reset(AnyReference(dstTypes[i],
                                                 uniqueConvertedField->rawValue()),
                                    false, uniqueConvertedField.ownsReference());
          uniqueConvertedField.release();
        }
      }

      // attempt both conversions
      if (!tsrc->convertTo(fields, fieldMissing, fieldDrop) &&
          !tdst->convertFrom(fields, fieldMissing, fieldDrop))
      {
        qiLogVerbose() << "Source and target cannot convert struct";
        return {};
      }
      // move stuff back to targetdata
      for (unsigned i = 0; i < dstNames.size(); ++i)
      {
        targetData[i] = fields[dstNames[i]].rawValue();
      }
    }

    void* dst = tdst->initializeStorage();
    tdst->set(&dst, targetData);
    return UniqueAnyReference{ AnyReference(tdst, dst) };
  }

  UniqueAnyReference AnyReferenceBase::convert(StructTypeInterface* tdst) const
  {
    if (!tdst)
      return {};

    switch (_type->kind())
    {
    case TypeKind_Tuple:
    {
      return ka::invoke_catch(DefaultUniqueAnyRef{}, [&] {
        StructTypeInterface* tsrc = static_cast<StructTypeInterface*>(_type);
        std::vector<void*> sourceData = tsrc->get(_value);
        std::vector<TypeInterface*> srcTypes = tsrc->memberTypes();
        std::vector<TypeInterface*> dstTypes = tdst->memberTypes();
        QI_ASSERT(sourceData.size() == srcTypes.size());
        if (srcTypes.size() != dstTypes.size())
        {
          qiLogVerbose() << "Conversion glitch: tuple size mismatch between " << tsrc->infoString() << " and " << tdst->infoString();
          return structConverter(this, tdst);
        }
        std::vector<std::string> srcNames = tsrc->elementsName();
        std::vector<std::string> dstNames = tdst->elementsName();
        if (srcNames.size() == srcTypes.size() && dstNames.size() == dstTypes.size())
        {
          std::sort(srcNames.begin(), srcNames.end());
          std::sort(dstNames.begin(), dstNames.end());
          if (srcNames != dstNames)
          {
            qiLogVerbose() << "Conversion glitch: names mismatch in named tuple";
            return structConverter(this, tdst);
          }
        }
        // Note: start converting without further check.
        // It means the case where a struct was modified but the
        // field count is unchanged will be badly suboptimal.
        // But further checks will degrade the nominal case.
        std::vector<UniqueAnyReference> uniqueConvertedFields;
        uniqueConvertedFields.reserve(dstTypes.size());
        std::vector<void*> targetData;
        targetData.reserve(dstTypes.size());
        for (unsigned i=0; i<dstTypes.size(); ++i)
        {
          auto conv = AnyReference(srcTypes[i], sourceData[i]).convert(dstTypes[i]);
          if (!conv->_type)
          {
            qiLogVerbose() << "Conversion failure in tuple member between "
                           << srcTypes[i]->infoString() << " and " << dstTypes[i]->infoString();
            return structConverter(this, tdst);
          }
          uniqueConvertedFields.emplace_back(std::move(conv));
          targetData.push_back(uniqueConvertedFields.back()->rawValue());
        }

        void* dst = tdst->initializeStorage();
        tdst->set(&dst, targetData);

        return UniqueAnyReference{ AnyReference{ tdst, dst } };
      });
    }
    case TypeKind_Map: {
      return ka::invoke_catch(DefaultUniqueAnyRef{}, [&]() -> UniqueAnyReference {
        MapTypeInterface* tsrc = static_cast<MapTypeInterface*>(_type);

        //key must be string
        if (tsrc->keyType()->kind() != TypeKind_String && tsrc->keyType()->kind() != TypeKind_Dynamic) {
          qiLogWarning() << "convert from map to struct, the key should be a string. (was " << tsrc->keyType()->kind() << ")";
          return {};
        }
        const std::vector<std::string>& elems = tdst->elementsName();
        std::vector<TypeInterface*> dstTypes = tdst->memberTypes();

        if (elems.size() != dstTypes.size()) {
          qiLogWarning() << "convert from map to struct, can't convert to tuple";
          return {};
        }

        std::vector<UniqueAnyReference> uniqueConvertedFields;
        uniqueConvertedFields.reserve(dstTypes.size());
        std::vector<void*> targetData { dstTypes.size(), nullptr };
        for (AnyIterator iter = tsrc->begin(_value), end = tsrc->end(_value);
            iter != end;
            ++iter)
        {
          std::string lname;
          try {
            lname = (*iter)[0].to<std::string>();
          }
          catch (std::exception& /* e */) {
            qiLogVerbose() << "can't convert map key " << (*iter)[0].type()->infoString() << " to string";
            continue;
          }

          std::vector<std::string>::const_iterator riter =
            std::find(elems.begin(), elems.end(), lname);
          if (riter == elems.end())
            continue;
          size_t rpos = riter-elems.begin();

          AnyReference ref = (*iter)[1];
          QI_ASSERT(ref.isValid());

          auto conv = ref.convert(dstTypes[rpos]);
          if (!conv->isValid())
          {
            qiLogWarning() << "convert from map to struct, cant convert to the right type for '" << *riter << "' from " << ref.type()->infoString() << " to " << dstTypes[rpos]->infoString();
            return {};
          }
          uniqueConvertedFields.emplace_back(std::move(conv));
          targetData[rpos] = uniqueConvertedFields.back()->rawValue();
        }

        if (uniqueConvertedFields.size() != elems.size())
        {
          qiLogWarning() << "convert from map to struct failed, some elements do not exist";
          return {};
        }

        void* dst = tdst->initializeStorage();
        tdst->set(&dst, targetData);
        return UniqueAnyReference{ AnyReference{ tdst, dst } };
      });
    }
    case TypeKind_VarArgs:
    case TypeKind_List:
    {
      return ka::invoke_catch(DefaultUniqueAnyRef{}, [&]() -> UniqueAnyReference {
        // No explicit type-check, convert will do it
        // handles for instance [i] -> (iii) if size matches
        // or [m] -> (anything) if effective types match
        ListTypeInterface* tsrc = static_cast<ListTypeInterface*>(_type);

        AnyIterator srcBegin = tsrc->begin(_value);
        AnyIterator srcEnd = tsrc->end(_value);

        std::vector<TypeInterface*> dstTypes = tdst->memberTypes();
        std::vector<UniqueAnyReference> uniqueConvertedFields;
        uniqueConvertedFields.reserve(dstTypes.size());
        std::vector<void*> targetData;
        targetData.reserve(dstTypes.size());

        if (this->size() != dstTypes.size())
        {
          qiLogWarning() << "Conversion failure: containers size mismatch between "
                         << tsrc->signature().toString() << " and " << tdst->signature().toString();
          return {};
        }

        unsigned int i = 0;
        while (srcBegin != srcEnd)
        {
          auto conv = (*srcBegin).convert(dstTypes[i]);
          if (!conv->isValid())
            return {};
          uniqueConvertedFields.emplace_back(std::move(conv));
          targetData.push_back(uniqueConvertedFields.back()->rawValue());
          ++srcBegin;
          ++i;
        }
        void* dst = tdst->initializeStorage();
        tdst->set(&dst, targetData);
        return UniqueAnyReference{ AnyReference{ tdst, dst } };
      });
    }
    default:
      break;
    }
    return {};
  }

  UniqueAnyReference AnyReferenceBase::convert(MapTypeInterface* targetType) const
  {
    if (!targetType)
      return {};

    switch (_type->kind())
    {
    case TypeKind_Map:
    {
      return ka::invoke_catch(DefaultUniqueAnyRef{}, [&]() -> UniqueAnyReference {
        UniqueAnyReference result{ AnyReference{ targetType } };
        MapTypeInterface* targetMapType = static_cast<MapTypeInterface*>(targetType);
        MapTypeInterface* srcMapType = static_cast<MapTypeInterface*>(_type);

        TypeInterface* srcKeyType = srcMapType->keyType();
        TypeInterface* srcElementType = srcMapType->elementType();

        TypeInterface* targetKeyType = targetMapType->keyType();
        TypeInterface* targetElementType = targetMapType->elementType();

        bool sameKey = srcKeyType->info() == targetKeyType->info();
        bool sameElem = srcElementType->info() == targetElementType->info();

        for (auto kv : *this)
        {
          UniqueAnyReference ck, cv;
          if (!sameKey)
          {
            ck = kv[0].convert(targetKeyType);
            if (!ck->_type)
              return {};
          }
          if (!sameElem)
          {
            cv = kv[1].convert(targetElementType);
            if (!cv->_type)
              return {};
          }
          result->insert(sameKey ? kv[0] : *ck, sameElem ? kv[1] : *cv);
        }
        return result;
      });
    }
    case TypeKind_VarArgs:
    case TypeKind_List:
    {
      return ka::invoke_catch(DefaultUniqueAnyRef{}, [&]() -> UniqueAnyReference {
        // Accept [(kv)] and convert to {kv}
        // Also accept [[m]] , [[k]] if k=v and size match, and other compatible stuffs
        UniqueAnyReference result{ AnyReference{ targetType } };
        ListTypeInterface* tsrc = static_cast<ListTypeInterface*>(_type);

        AnyIterator srcBegin = tsrc->begin(_value);
        AnyIterator srcEnd = tsrc->end(_value);

        std::vector<TypeInterface*> vectorValueType;
        vectorValueType.push_back(static_cast<MapTypeInterface*>(targetType)->keyType());
        vectorValueType.push_back(static_cast<MapTypeInterface*>(targetType)->elementType());

        TypeInterface* pairType = makeTupleType(vectorValueType);

        while (srcBegin != srcEnd)
        {
          auto conv = (*srcBegin).convert(pairType);
          if (!conv->_type)
          {
            return {};
          }
          result->insert((*conv)[0], (*conv)[1]);
          ++srcBegin;
        }
        return result;
      });
    }
    case TypeKind_Tuple:
    {
      return ka::invoke_catch(DefaultUniqueAnyRef{}, [&] () -> UniqueAnyReference{
        UniqueAnyReference result{ AnyReference{ targetType } };
        auto srcStructType = static_cast<StructTypeInterface*>(_type);
        MapTypeInterface* targetMapType = targetType;

        // Source fields value
        std::vector<void*> sourceData = srcStructType->get(_value);
        // Source fields name
        std::vector<std::string> srcElementName = srcStructType->elementsName();
        // Source members type
        std::vector<TypeInterface*> srcTypes = srcStructType->memberTypes();
        // Destination members type
        TypeInterface* dstType = targetMapType->elementType();

        // trying to convert std::pair to std::map
        if (srcElementName.size() != srcTypes.size())
          return {};

        for (unsigned int i = 0; i < srcElementName.size(); ++i)
        {
          auto conv = AnyReference(srcTypes[i], sourceData[i]).convert(dstType);
          if (!conv->_type)
          {
            qiLogVerbose() << "Conversion failure in tuple member between "
                           << srcTypes[i]->infoString() << " and " << dstType->infoString();
            return {};
          }
          result->insert(AnyReference::from(srcElementName[i]), *conv);
        }
        return result;
      });
    }
    default:
      break;
    }

    return {};
  }

  std::pair<char*, size_t> AnyReferenceBase::asRaw() const {
    if (kind() != TypeKind_Raw)
      throw std::runtime_error("asRaw only available for raw kind");
    return static_cast<RawTypeInterface*>(_type)->get(_value);
  }

  boost::optional<AnyReference> AnyReferenceBase::asOptional() const
  {
    if (kind() != TypeKind_Optional)
      throw std::runtime_error("asOptional only available for optional kind");
    if (optionalHasValue())
      return boost::make_optional(content());
    return {};
  }

  UniqueAnyReference AnyReferenceBase::convert(TypeInterface* targetType) const
  {
    /* Can have false-negative (same effective type, different Type instances
     * but we do not care, correct check (by comparing info() result
     * is more expensive than the dummy conversion that will happen.
     */
    if (!targetType || !_type) {
      qiLogWarning() << "Conversion error: can't convert to/from a null type.";
      return {};
    }
    qiLogDebug() << "convert "
      << _type->infoString() << '(' << _type->kind() << ") "
      << targetType->infoString() << '(' << targetType->kind() << ')' ;

    if (_type == targetType)
      return UniqueAnyReference{ *this, DeferOwnership{} };

    TypeKind skind = _type->kind();
    TypeKind dkind = targetType->kind();

    if (skind == dkind)
    {
      switch(dkind)
      {
      case TypeKind_Void:
        return UniqueAnyReference{ qi::AnyReference(targetType) };
      case TypeKind_Float:
        return convert(static_cast<FloatTypeInterface*>(targetType));
      case TypeKind_Int:
        return convert(static_cast<IntTypeInterface*>(targetType));
      case TypeKind_String:
        return convert(static_cast<StringTypeInterface*>(targetType));
      case TypeKind_VarArgs:
        return convert(static_cast<ListTypeInterface*>(targetType));
      case TypeKind_List:
        return convert(static_cast<ListTypeInterface*>(targetType));
      case TypeKind_Map:
        return convert(static_cast<MapTypeInterface*>(targetType));
      case TypeKind_Pointer:
        return convert(static_cast<PointerTypeInterface*>(targetType));
      case TypeKind_Tuple:
        return convert(static_cast<StructTypeInterface*>(targetType));
      case TypeKind_Dynamic:
        return convert(static_cast<DynamicTypeInterface*>(targetType));
      case TypeKind_Raw:
        return convert(static_cast<RawTypeInterface*>(targetType));
      case TypeKind_Optional:
        return convert(static_cast<OptionalTypeInterface*>(targetType));
      case TypeKind_Unknown:
      {
        /* Under clang macos, typeInfo() comparison fails
           * for non-exported (not forced visibility=default since we default to hidden)
           * symbols. So ugly hack, compare the strings.
           */
        if (targetType->info() == _type->info()
    #ifdef __clang__
            || targetType->info().asString() ==  _type->info().asString()
    #endif
            )
          return UniqueAnyReference{ *this, DeferOwnership{} };
        else
          return {};
      }
      default:
        break;
      }
    }
    // skind != dkind
    if (skind == TypeKind_List && dkind == TypeKind_Tuple)
      return convert(static_cast<StructTypeInterface*>(targetType));
    else if (skind == TypeKind_Tuple && dkind == TypeKind_List)
      return convert(static_cast<ListTypeInterface*>(targetType));
    else if (skind == TypeKind_Tuple && dkind == TypeKind_Map)
      return convert(static_cast<MapTypeInterface*>(targetType));

    else if (skind == TypeKind_VarArgs && dkind == TypeKind_List)
      return convert(static_cast<ListTypeInterface*>(targetType));
    else if (skind == TypeKind_List && dkind == TypeKind_VarArgs)
      return convert(static_cast<ListTypeInterface*>(targetType));

    else if (skind == TypeKind_VarArgs && dkind == TypeKind_Tuple)
      return convert(static_cast<StructTypeInterface*>(targetType));
    else if (skind == TypeKind_Tuple && dkind == TypeKind_VarArgs)
      return convert(static_cast<ListTypeInterface*>(targetType));


    else if (skind == TypeKind_List && dkind == TypeKind_Map)
      return convert(static_cast<MapTypeInterface*>(targetType));
    else if (skind == TypeKind_Map && dkind == TypeKind_List)
      return convert(static_cast<ListTypeInterface*>(targetType));
    else if (skind == TypeKind_Map && dkind == TypeKind_Tuple)
      return convert(static_cast<StructTypeInterface*>(targetType));


    else if (skind == TypeKind_Float && dkind == TypeKind_Int)
      return convert(static_cast<IntTypeInterface*>(targetType));
    else if (skind == TypeKind_Int && dkind == TypeKind_Float)
      return convert(static_cast<FloatTypeInterface*>(targetType));

    else if (skind == TypeKind_String && dkind == TypeKind_Raw)
      return convert(static_cast<RawTypeInterface*>(targetType));
    else if (skind == TypeKind_Raw && dkind == TypeKind_String)
      return convert(static_cast<StringTypeInterface*>(targetType));

    if (targetType->info() == typeOf<AnyObject>()->info()
        && _type->kind() == TypeKind_Pointer
        && static_cast<PointerTypeInterface*>(_type)->pointedType()->kind() == TypeKind_Object)
    { // Pointer to concrete object -> AnyObject
      // Keep a copy of this in AnyObject, and destroy on AnyObject destruction
      // That way if this is a shared_ptr, we link to it correctly
      PointerTypeInterface* pT = static_cast<PointerTypeInterface*>(_type);
      auto pointee = pT->dereference(_value);
      auto* pointeeType = static_cast<ObjectTypeInterface*>(pointee.type());
      QI_ASSERT_TRUE(pointeeType);
      auto* pointeeValue = pointee.rawValue();
      QI_ASSERT_TRUE(pointeeValue);
      const ObjectUid pointeeUid = pointeeType->uid(pointeeValue);
      AnyValue pointer{*this};
      auto go =
        detail::ManagedObjectPtr(new GenericObject(pointeeType, pointeeValue, pointeeUid),
                                 [pointer](GenericObject* obj) mutable {
                                   delete obj;
                                   pointer.reset(); // reset explicitly because we don't want the
                                                    // pointer's lifetime be that of the deleter
                                 });
      AnyObject obj(std::move(go));

      return UniqueAnyReference{ AnyReference::from(obj).clone() };
    }

    if (_type->info() == typeOf<AnyObject>()->info()
        && targetType->kind() == TypeKind_Pointer)
    {
      // if pointer is the exact pointer, use it
      PointerTypeInterface* pT = static_cast<PointerTypeInterface*>(targetType);
      AnyObject* self = static_cast<AnyObject*>(_value);
      if (self->asGenericObject()->type->info() == pT->pointedType()->info())
      {
        AnyReference res(pT);
        pT->set(&res._value, qi::AnyReference::from(boost::static_pointer_cast<void>(self->asSharedPtr())));
        return UniqueAnyReference{ res };
      }

      // Attempt specialized proxy conversion
      qiLogDebug() << "Attempting specialized proxy conversion";
      detail::ProxyGeneratorMap& map = detail::proxyGeneratorMap();
      detail::ProxyGeneratorMap::iterator it = map.find(
                                                 static_cast<PointerTypeInterface*>(targetType)->pointedType()->info());
      if (it != map.end())
      {
        return UniqueAnyReference{ (it->second)(*(AnyObject*)_value) };
      }
      else
        qiLogDebug() << "type "
                     << static_cast<PointerTypeInterface*>(targetType)->pointedType()->infoString()
                     <<" not found in proxy map";
    }

    if (targetType->kind() == TypeKind_Dynamic)
      return convert(static_cast<DynamicTypeInterface*>(targetType));

    if (_type->kind() == TypeKind_Dynamic)
    {
      AnyReference gv = content();
      return gv.convert(targetType);
    }

    if (targetType->kind() == TypeKind_Optional)
    {
      return convert(static_cast<OptionalTypeInterface*>(targetType));
    }

    if (skind == TypeKind_Object && dkind == TypeKind_Pointer)
      return convert(static_cast<PointerTypeInterface*>(targetType));

    if (skind == TypeKind_Object)
    {
      // Try inheritance
      ObjectTypeInterface* osrc = static_cast<ObjectTypeInterface*>(_type);
      qiLogDebug() << "inheritance check "
        << osrc <<" " << (osrc?osrc->inherits(targetType):false);
      std::ptrdiff_t inheritOffset = 0;
      if (osrc && (inheritOffset = osrc->inherits(targetType)) != ObjectTypeInterface::INHERITS_FAILED)
      {
        // We return a Value that point to the same data as this.
        return UniqueAnyReference{ AnyReference{ targetType,
                                                 (void*)((intptr_t)_value + inheritOffset) },
                                   DeferOwnership{} };
      }
    }

    if (_type->info() == targetType->info())
      return UniqueAnyReference{ *this, DeferOwnership{} };

    return {};
  }

  AnyReference AnyReferenceBase::convertCopy(TypeInterface* targetType) const
  {
    auto res = convert(targetType);
    if (res.ownsReference())
      return res.release();
    else
      return res->clone();
  }

}

bool operator<(const AnyReference& a, const AnyReference& b)
{
  //qiLogDebug() << "Compare " << a._type << ' ' << b._type;
#define GET(v, t) static_cast< t ## TypeInterface *>(v.type())->get(v.rawValue())
  if (!a.type())
    return b.type() != 0;
  if (!b.type())
    return false;
  /* < operator for char* does not do what we want, so force
  * usage of get() below for string types.
  */
  if ((a.type() == b.type() || a.type()->info() == b.type()->info())
      && a.type()->kind() != TypeKind_String)
  {
    //qiLogDebug() << "Compare sametype " << a._type->infoString();
    return a.type()->less(a.rawValue(), b.rawValue());
  }
  // Comparing values of different types
  TypeKind ka = a.type()->kind();
  TypeKind kb = b.type()->kind();
  //qiLogDebug() << "Compare " << ka << ' ' << kb;
  if (ka != kb)
  {
    if (ka == TypeKind_Int && kb == TypeKind_Float)
      return GET(a, Int) < GET(b, Float);
    else if (ka == TypeKind_Float && kb == TypeKind_Int)
      return GET(a, Float) < GET(b, Int);
    else
      return ka < kb; // Safer than comparing pointers
  }
  else switch(ka)
  {
  case TypeKind_Void:
    return false;
  case TypeKind_Int:
    return GET(a, Int) < GET(b, Int);
  case TypeKind_Float:
    return GET(a, Float) < GET(b, Float);
  case TypeKind_String:
  {
    StringTypeInterface::ManagedRawString ca, cb;
    ca = GET(a, String);
    cb = GET(b, String);
    bool res = ca.first.second == cb.first.second ?
      (memcmp(ca.first.first, cb.first.first, ca.first.second) < 0) :
      (ca.first.second < cb.first.second);
    if (ca.second)
      ca.second(ca.first);
    if (cb.second)
      cb.second(cb.first);
    qiLogDebug() << "Compare " << ca.first.first << ' ' <<
      cb.first.first << ' ' << res;
    return res;
  }
  case TypeKind_VarArgs:
  case TypeKind_List:
  case TypeKind_Map: // omg, same code!
  {
    size_t la = a.size();
    size_t lb = b.size();
    if (la != lb)
      return la < lb;
    AnyIterator ita   = a.begin();
    AnyIterator enda = a.end();
    AnyIterator itb   = b.begin();
    AnyIterator endb = b.end();
    while (ita != enda)
    {
      assert (! (itb == endb));
      AnyReference ea = *ita;
      AnyReference eb = *itb;
      if (ea < eb)
        return true;
      else if (eb < ea)
        return false;
      ++ita;
      ++itb;
    }
    return false; // list are equals
  }
  case TypeKind_Object:
  case TypeKind_Pointer:
  case TypeKind_Tuple:
  case TypeKind_Dynamic:
  case TypeKind_Raw:
  case TypeKind_Unknown:
  case TypeKind_Iterator:
  case TypeKind_Function:
  case TypeKind_Signal:
  case TypeKind_Property:
  case TypeKind_Optional:
    return a.rawValue() < b.rawValue();
  }
#undef GET
  return a.rawValue() < b.rawValue();
}

bool operator==(const AnyReference& a, const AnyReference& b)
{
  if (!a.isValid() || !b.isValid())
    return !a.isValid() && !b.isValid();

  if (a.kind() == TypeKind_Iterator && b.kind() == TypeKind_Iterator
      && a.type()->info() == b.type()->info())
  {
    return static_cast<IteratorTypeInterface*>(a.type())->equals(a.rawValue(), b.rawValue());
  }
  else
    return ! (a < b) && !(b < a);
}

namespace detail
{

  AnyValue AnyReferenceBase::toTuple(bool homogeneous) const
  {
    if (kind() == TypeKind_Tuple)
      return AnyValue(*this);
    else if (kind() != TypeKind_List && kind() != TypeKind_VarArgs)
      throw std::runtime_error("Expected Tuple or List kind");
    // convert list to tuple

    ListTypeInterface* t = static_cast<ListTypeInterface*>(_type);
    TypeInterface* te = t->elementType();
    DynamicTypeInterface* td = nullptr;
    if (te->kind() == TypeKind_Dynamic)
      td = static_cast<DynamicTypeInterface*>(te);
    if (!homogeneous && !td)
      throw std::runtime_error("Element type is not dynamic");
    AnyReferenceVector elems;
    AnyIterator it = begin();
    AnyIterator iend = end();
    while (it != iend)
    {
      AnyReference e = *it;
      if (homogeneous)
        elems.push_back(e);
      else
        elems.push_back(e.content());
      ++it;
    }

    //makeGenericTuple allocates, steal the result
    return AnyValue(makeGenericTuple(elems), false, true);
  }

  AnyObject AnyReferenceBase::toObject() const
  {
    return to<AnyObject>();
  }

  AnyReference AnyReferenceBase::_element(const AnyReference& key, bool throwOnFailure, bool autoInsert)
  {
    if (kind() == TypeKind_List || kind() == TypeKind_VarArgs)
    {
      ListTypeInterface* t = static_cast<ListTypeInterface*>(_type);
      int ikey = (int)key.toInt();
      if (ikey < 0 || static_cast<size_t>(ikey) >= t->size(_value))
      {
        if (throwOnFailure)
          throw std::runtime_error("Index out of range");
        else
          return AnyReference();
      }
      return AnyReference(t->elementType(), t->element(_value, ikey));
    }
    else if (kind() == TypeKind_Map)
    {
      MapTypeInterface* t = static_cast<MapTypeInterface*>(_type);
      auto c = key.convert(t->keyType());
      if (!c->_type)
        throw std::runtime_error("Incompatible key type");
      return t->element(&_value, c->_value, autoInsert);
    }
    else if (kind() == TypeKind_Tuple)
    {
      StructTypeInterface* t = static_cast<StructTypeInterface*>(_type);
      int ikey = (int)key.toInt();
      std::vector<TypeInterface*> types = t->memberTypes();
      if (ikey < 0 || static_cast<size_t>(ikey) >= types.size())
      {
        if (throwOnFailure)
          throw std::runtime_error("Index out of range");
        else
          return AnyReference();
      }
      return AnyReference(types[ikey], t->get(_value, ikey));
    }
    else
      throw std::runtime_error("Expected List, Map or Tuple kind");
  }

  void AnyReferenceBase::append(const AnyReference& elem)
  {
    if (kind() != TypeKind_List && kind() != TypeKind_VarArgs)
      throw std::runtime_error("Expected a list");
    ListTypeInterface* t = static_cast<ListTypeInterface*>(_type);
    auto c = elem.convert(t->elementType());

    if (!c->isValid())
      throwConversionFailure(elem._type, t->elementType(), "(invalid value type)");

    t->pushBack(&_value, c->_value);
  }

  void AnyReferenceBase::insert(const AnyReference& key, const AnyReference& val)
  {
    if (kind() != TypeKind_Map)
      throw std::runtime_error("Expected a map");
    MapTypeInterface* t = static_cast<MapTypeInterface*>(_type);

    UniqueAnyReference ck{ key, DeferOwnership{} };
    UniqueAnyReference cv{ val, DeferOwnership{} };

    if (key._type != t->keyType())
      ck = key.convert(t->keyType());
    if (!ck->isValid())
      throwConversionFailure(key._type, t->keyType(), "(invalid key type)");

    if (val._type != t->elementType())
      cv = val.convert(t->elementType());
    if (!cv->isValid())
      throwConversionFailure(val._type, t->elementType(), "(invalid value type)");

    t->insert(&_value, ck->_value, cv->_value);
  }

  void AnyReferenceBase::update(const AutoAnyReference& val)
  {
    switch(val.kind())
    {
    case TypeKind_Int:
      setInt(val.toInt());
      break;
    case TypeKind_Float:
      setDouble(val.toDouble());
      break;
    case TypeKind_String:
      setString(val.toString());
      break;
    case TypeKind_Dynamic:
      setDynamic(val);
      break;
    case TypeKind_Tuple:
      //asTupleValuePtr is not const, so copy val to a non-const AnyReference
      setTuple(AnyReference(val).asTupleValuePtr());
      break;
    case TypeKind_Raw: {
        std::pair<char*, size_t> pa = val.asRaw();
        setRaw(pa.first, pa.second);
        break;
      }
    case TypeKind_Optional:
      setOptional(AnyReference(val).asOptional());
      break;
    default:
      throw std::runtime_error("Update not implemented for this type.");
    }
  }

  void AnyReferenceBase::setInt(int64_t v)
  {
    if (kind() == TypeKind_Int)
    {
      IntTypeInterface* type = static_cast<IntTypeInterface*>(this->_type);
      if (!type->isSigned() && v < 0)
        throw std::runtime_error(_QI_LOG_FORMAT("Converting negative value %s to unsigned type", v));
      // not signed gives us an extra bit, but signed can go down an extra value
      if (type->size() > 8)
        throw std::runtime_error(_QI_LOG_FORMAT("Overflow converting %s to %s bytes", v, type->size()));
      if (type->size() == 0 && (v < 0 || v > 1))
        throw std::runtime_error(_QI_LOG_FORMAT("Expected 0 or 1 when converting to bool, got %s", v));
      if (type->size() > 0 && type->size() < 8 && (std::abs(v) >= (1LL << (8*type->size() - (type->isSigned()?1:0))) + ((v<0)?1:0)))
        throw std::runtime_error(_QI_LOG_FORMAT("Overflow converting %s to %s bytes", v, type->size()));
      type->set(&_value, v);
    }
    else if (kind() == TypeKind_Float)
      static_cast<FloatTypeInterface*>(_type)->set(&_value, static_cast<double>(v));
    else
      throw std::runtime_error("Value is not Int or Float");
  }

  void AnyReferenceBase::setDynamic(const qi::AnyReference &element) {
    if (kind() != TypeKind_Dynamic)
      throw std::runtime_error("Value is not a Dynamic");
    DynamicTypeInterface* t = static_cast<DynamicTypeInterface*>(this->_type);
    t->set(&_value, element);
  }

  void AnyReferenceBase::setOptional(const boost::optional<AnyReference>& opt)
  {
    if (kind() != TypeKind_Optional)
      throw std::runtime_error("Cannot set optional from argument: object is not an Optional");

    auto* const thisType = static_cast<OptionalTypeInterface*>(_type);

    if (!opt)
      thisType->reset(&_value);
    else
    {
      const auto& argValue = *opt;
      auto conv = argValue.convert(thisType->valueType());
      if (!conv->isValid())
        throwConversionFailure(argValue.type(), thisType, "(invalid optional value type)");
      thisType->set(&_value, conv->_value);
    }
  }

  void AnyReferenceBase::setOptional(const AnyReference& opt)
  {
    return setOptional(opt.asOptional());
  }

  void AnyReferenceBase::setRaw(const char *buffer, size_t size) {
    if (kind() != TypeKind_Raw)
      throw std::runtime_error("Value is not a Raw");
    static_cast<RawTypeInterface*>(_type)->set(&_value, buffer, size);
  }

  void AnyReferenceBase::setUInt(uint64_t v)
  {
    if (kind() == TypeKind_Int)
    {
      IntTypeInterface* type = static_cast<IntTypeInterface*>(this->_type);
      if (type->size() > 0 && type->size() < 8 && (v >= (1ULL << (8*type->size() - (type->isSigned()?1:0)))))
        throw std::runtime_error(_QI_LOG_FORMAT("Overflow converting %s to %s bytes", v, type->size()));
      if (type->size() == 0 && (v > 1))
        throw std::runtime_error(_QI_LOG_FORMAT("Expected 0 or 1 when converting to bool, got %s", v));
      if (type->size() == 8 && type->isSigned() && v >= 0x8000000000000000ULL)
        throw std::runtime_error(_QI_LOG_FORMAT("Overflow converting %s to signed int64", v));
      type->set(&_value, (int64_t)v);
    }
    else if (kind() == TypeKind_Float)
      static_cast<FloatTypeInterface*>(_type)->set(&_value,
        static_cast<double>(static_cast<uint64_t>(v)));
    else
      throw std::runtime_error("Value is not Int or Float");
  }

  void AnyReferenceBase::setDouble(double v)
  {
    if (kind() == TypeKind_Float)
      static_cast<FloatTypeInterface*>(_type)->set(&_value, v);
    else if (kind() == TypeKind_Int)
    {
      IntTypeInterface* type = static_cast<IntTypeInterface*>(this->_type);
      if (v < 0 && !type->isSigned())
        throw std::runtime_error(_QI_LOG_FORMAT("Converting negative value %s to unsigned type", v));
      if (type->size() == 0 && std::min(std::abs(v), std::abs(v-1)) > 0.01)
        throw std::runtime_error(_QI_LOG_FORMAT("Expected 0 or 1 when converting to bool, got %s", v));
      if (type->size() != 0 && type->size() < 8 && (std::abs(v) >= (1ULL << (8*type->size() - (type->isSigned()?1:0))) + ((v<0)?1:0)))
        throw std::runtime_error(_QI_LOG_FORMAT("Overflow converting %s to %s bytes", v, type->size()));
      if (type->size() == 8
          && std::abs(v) > (type->isSigned()?
            (double)std::numeric_limits<int64_t>::max()
            :(double)std::numeric_limits<uint64_t>::max()))
        throw std::runtime_error(_QI_LOG_FORMAT("Overflow converting %s to %s bytes", v, type->size()));
      type->set(&_value, static_cast<int64_t>(v));
    }
    else
      throw std::runtime_error("Value is not Int or Float");
  }

  AnyReferenceVector AnyReferenceBase::asTupleValuePtr()
  {
    if (kind() == TypeKind_Tuple)
      return static_cast<StructTypeInterface*>(_type)->values(_value);
    else if (kind() == TypeKind_List || kind() == TypeKind_VarArgs || kind() == TypeKind_Map)
    {
      AnyReferenceVector result;
      for(auto elem : *this)
        result.push_back(elem);
      return result;
    }
    else if (kind() == TypeKind_Dynamic)
    {
      return (**this).asTupleValuePtr();
    }
    else
      throw std::runtime_error("Expected tuple, list or map");
  }

  AnyIterator AnyReferenceBase::begin() const
  {
    if (kind() == TypeKind_List || kind() == TypeKind_VarArgs)
      return static_cast<ListTypeInterface*>(_type)->begin(_value);
    else if (kind() == TypeKind_Map)
      return static_cast<MapTypeInterface*>(_type)->begin(_value);
    else
      throw std::runtime_error("Expected list or map");
  }

  AnyIterator AnyReferenceBase::end() const
  {
    if (kind() == TypeKind_List || kind() == TypeKind_VarArgs)
      return static_cast<ListTypeInterface*>(_type)->end(_value);
    else if (kind() == TypeKind_Map)
      return static_cast<MapTypeInterface*>(_type)->end(_value);
    else
      throw std::runtime_error("Expected list or map");
  }

  std::map<AnyReference, AnyReference> AnyReferenceBase::asMapValuePtr()
  {
    if (kind() != TypeKind_Map)
      throw std::runtime_error("Expected a map");
    std::map<AnyReference, AnyReference> result;
    for(auto elem : *this)
      result[elem[0]] = elem[1];
    return result;
  }

  void AnyReferenceBase::setString(const std::string& v)
  {
    if (kind() != TypeKind_String)
      throw std::runtime_error("Value is not of kind string");
    static_cast<StringTypeInterface*>(_type)->set(&_value, &v[0], v.size());
  }

  void AnyReferenceBase::setTuple(const AnyReferenceVector& values) {
    if (kind() != TypeKind_Tuple)
      throw std::runtime_error("Value is not a Tuple");
    StructTypeInterface* stype = static_cast<StructTypeInterface*>(_type);
    std::vector<TypeInterface*> types = stype->memberTypes();
    std::vector<void*> vals;

    if (types.size() != values.size())
      throw std::runtime_error(_QI_LOG_FORMAT("Can't change values of the tuple, size mismatch (%d != %d)", types.size(), values.size()));
    vals.resize(types.size());
    for (unsigned int i = 0; i < values.size(); ++i) {
      //TODO: implement conversion here
      if (types[i]->info() == values[i].type()->info())
        vals[i] = values[i].rawValue();
      else
        throw std::runtime_error("Type mismatchs when assigning values to tuple");
    }
    stype->set(&_value, vals);
  }

  void AnyReferenceBase::resetOptional()
  {
    if (kind() != TypeKind_Optional)
      throw std::runtime_error("Value is not an optional");

    const auto optType = static_cast<OptionalTypeInterface*>(_type);
    optType->reset(&_value);
  }

  size_t AnyReferenceBase::size() const
  {
    if (kind() == TypeKind_List || kind() == TypeKind_VarArgs)
      return static_cast<ListTypeInterface*>(_type)->size(_value);
    if (kind() == TypeKind_Map)
      return static_cast<MapTypeInterface*>(_type)->size(_value);
    if (kind() == TypeKind_Tuple)
      return static_cast<StructTypeInterface*>(_type)->memberTypes().size();
    else
      throw std::runtime_error("Expected List, Map or Tuple.");
  }

  bool AnyReferenceBase::optionalHasValue() const
  {
    if (kind() != TypeKind_Optional)
      throw std::runtime_error("Expected Optional.");
    return static_cast<OptionalTypeInterface*>(_type)->hasValue(_value);
  }

  AnyReference AnyReferenceBase::content() const
  {
    if (kind() == TypeKind_Pointer)
      return static_cast<PointerTypeInterface*>(_type)->dereference(_value);
    else if (kind() == TypeKind_Iterator)
      return static_cast<IteratorTypeInterface*>(_type)->dereference(_value);
    else if (kind() == TypeKind_Dynamic)
      return static_cast<DynamicTypeInterface*>(_type)->get(_value);
    else if (kind() == TypeKind_Optional)
      return static_cast<OptionalTypeInterface*>(_type)->value(_value);
    else
      throw std::runtime_error("Expected pointer, dynamic, iterator or optional");
  }

  AnyReference AnyReferenceBase::operator*() const
  {
    return content();
  }

  std::vector<TypeInterface*> AnyReferenceBase::membersType() const
  {
    if (kind() == TypeKind_Tuple)
      return static_cast<StructTypeInterface*>(_type)->memberTypes();
    else
      throw std::runtime_error("Expected tuple");
  }


  QI_NORETURN void throwConversionFailure(TypeInterface* from, TypeInterface* to, const std::string& additionalMsg)
  {
    std::stringstream msg;
    msg << "Conversion from ";
    if (from) {
      msg << from->signature().toString() << '(' << from->info().asDemangledString() << ')';
    } else {
      msg << "NULL Type";
    }
    msg << " to ";
    if (to) {
      msg << to->signature().toString() << '(' << to->info().asDemangledString() << ')';
    } else {
      msg << "NULL Type";
    }
    msg << " failed ";
    msg << additionalMsg;
    qiLogDebug() << msg.str();
    throw std::runtime_error(msg.str());
  }

} // namespace detail

} // namespace qi
