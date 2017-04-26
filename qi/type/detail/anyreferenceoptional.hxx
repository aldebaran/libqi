/*
**  Copyright (C) 2018 SoftBank Robotics Europe
**  See COPYING for the license
*/

#ifndef QI_TYPE_DETAIL_ANYREFERENCEOPTIONAL_HXX
#define QI_TYPE_DETAIL_ANYREFERENCEOPTIONAL_HXX

#pragma once

#include <qi/type/detail/anyreference.hpp>
#include <qi/type/typeinterface.hpp>
#include <ka/scoped.hpp>
#include <qi/assert.hpp>

namespace qi
{
namespace detail
{

template<typename Proc>
void setOptionalValueReference(OptionalTypeInterface* type, void* value, Proc setValue)
{
  QI_ASSERT_TRUE(type);
  QI_ASSERT_TRUE(value);
  auto optIsSet = type->hasValue(value);
  auto ref = optIsSet ? type->value(value) : AnyReference{ type->valueType() };
  auto scopeDestroy = ka::scoped([&]{
    if (!optIsSet)
      ref.destroy();
  });
  setValue(ref); // if this throws, the optional value will not be affected
  if (!optIsSet)
  {
    type->set(&value, ref.rawValue());
    optIsSet = true;
  }
}

template<typename Proc>
void setOptionalValueReference(AnyReference optRef, Proc setValue)
{
  if (optRef.kind() != TypeKind_Optional)
    return;
  setOptionalValueReference(static_cast<OptionalTypeInterface*>(optRef._type), optRef._value,
                            std::move(setValue));
}

} // namespace detail

} // namespace qi

#endif // QI_TYPE_DETAIL_ANYREFERENCEOPTIONAL_HXX
