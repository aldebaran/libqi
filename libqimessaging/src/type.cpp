/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#include <qimessaging/type.hpp>


namespace qi {


  class TypeInfoKey
  {
  public:
    TypeInfoKey(const std::type_info& ti): ti(ti) {}
    bool operator < (const TypeInfoKey& b) const
    {
      return ti.before(b.ti);
    }
    bool operator == (const TypeInfoKey& b) const
    {
      return ti == b.ti;
    }
    const std::type_info& ti;
  };

  typedef std::map<TypeInfoKey, Type*> TypeFactory;
  static TypeFactory& typeFactory()
  {
    static TypeFactory* res = 0;
    if (!res)
      res = new TypeFactory;
    return *res;
  }

  QIMESSAGING_API Type* getType(const std::type_info& type)
  {
    // We create-if-not-exist on purpose: to detect access that occur before
    // registration
    return typeFactory()[TypeInfoKey(type)];
  }

  /// Type factory setter
  QIMESSAGING_API bool registerType(const std::type_info& typeId, Type* type)
  {
    qiLogDebug("qi.meta") << "registerType "  << typeId.name() << " " << (void*)type;
    TypeFactory::iterator i = typeFactory().find(TypeInfoKey(typeId));
    if (i != typeFactory().end())
    {
      if (i->second)
        qiLogWarning("qi.meta") << "registerType: previous registration present for "
          << typeId.name()<< " " << (void*)i->second;
      else
        qiLogWarning("qi.meta") << "registerType: access to type factory before"
          " registration detected for type " << typeId.name();
    }
    typeFactory()[TypeInfoKey(typeId)] = type;
    return true;
  }
}



