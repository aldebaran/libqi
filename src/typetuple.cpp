/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include <qitype/type.hpp>
#include <qitype/genericvalue.hpp>

namespace qi
{
  std::vector<GenericValuePtr> TypeTuple::getValues(void* storage)
  {
    std::vector<Type*> types = memberTypes(storage);
    std::vector<void*> values = get(storage);
    std::vector<GenericValuePtr> result;
    for (unsigned i=0; i<types.size(); ++i)
      result.push_back(GenericValuePtr(types[i], values[i]));
    return result;
  }

  std::vector<void*> TypeTuple::get(void* storage)
  {
    std::vector<void*> result;
    unsigned count = memberTypes(storage).size();
    for (unsigned i=0; i<count; ++i)
      result.push_back(get(storage, i));
    return result;
  }

  void TypeTuple::set(void** storage, std::vector<void*> values)
  {
    for (unsigned i=0; i<values.size(); ++i)
      set(storage, i, values[i]);
  }

}
