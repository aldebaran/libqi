/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include <qitype/type.hpp>
#include <qitype/genericvalue.hpp>

namespace qi
{


  namespace detail {
    std::string normalizeClassName(const std::string &name) {
      //::qi::Foo -> Foo
      size_t id = name.rfind("::");
      if (id != std::string::npos)
        return name.substr(id + 2);
      else return name;
    }
  }

  std::vector<GenericValuePtr> StructTypeInterface::values(void* storage)
  {
    std::vector<Type*> types = memberTypes();
    std::vector<void*> values = get(storage);
    std::vector<GenericValuePtr> result;
    for (unsigned i=0; i<types.size(); ++i)
      result.push_back(GenericValuePtr(types[i], values[i]));
    return result;
  }

  std::vector<void*> StructTypeInterface::get(void* storage)
  {
    std::vector<void*> result;
    unsigned count = memberTypes().size();
    for (unsigned i=0; i<count; ++i)
      result.push_back(get(storage, i));
    return result;
  }

  void StructTypeInterface::set(void** storage, std::vector<void*> values)
  {
    for (unsigned i=0; i<values.size(); ++i)
      set(storage, i, values[i]);
  }

}
