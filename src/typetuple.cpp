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

  class TypeDynamicTuple: public TypeTuple
  {
  public:
    virtual std::vector<Type*> memberTypes(void* storage)
    {
      std::vector<Type*> result;
      std::vector<GenericValuePtr>& me =
      *(std::vector<GenericValuePtr>*)ptrFromStorage(&storage);
      for (unsigned i=0; i< me.size(); ++i)
        result.push_back(me[i].type);
      return result;
    }
    virtual void* get(void* storage, unsigned int index)
    {
      std::vector<GenericValuePtr>& me =
      *(std::vector<GenericValuePtr>*)ptrFromStorage(&storage);
      return me[index].value;
    }
    virtual void set(void** storage, unsigned int index, void* val)
    {
      std::vector<GenericValuePtr>& me =
      *(std::vector<GenericValuePtr>*)ptrFromStorage(storage);
      me[index].value = val;
    }
    typedef DefaultTypeImplMethods<std::vector<GenericValuePtr> > Methods;
    _QI_BOUNCE_TYPE_METHODS(Methods);
  };

  GenericValuePtr makeGenericTuple(std::vector<GenericValuePtr> values)
  {
    static Type* dtype = 0;
    if (!dtype)
      dtype = new TypeDynamicTuple();
    GenericValuePtr res = GenericValuePtr::from(values);
    res.type = dtype;
    return res;
  }
}
