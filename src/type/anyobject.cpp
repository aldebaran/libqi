/*
**  Copyright (C) 2012, 2013 Aldebaran Robotics
**  See COPYING for the license
*/
#include <iostream>

#include <qi/anyobject.hpp>

#ifdef _MSC_VER
#  pragma warning( push )
#  pragma warning( disable: 4355 )
#endif

qiLogCategory("qitype.object");

namespace qi
{

int ObjectTypeInterface::inherits(TypeInterface* other)
{
  /* A registered class C can have to TypeInterface* around:
  * - TypeImpl<C*>
  * - The staticObjectTypeInterface that was created by the builder.
  * So assume that any of them can be in the parentTypes list.
  */
  if (this == other)
    return 0;
  const std::vector<std::pair<TypeInterface*, int> >& parents = parentTypes();
  qiLogDebug() << infoString() <<" has " << parents.size() <<" parents";
  for (unsigned i=0; i<parents.size(); ++i)
  {
    if (parents[i].first->info() == other->info())
      return parents[i].second;
    ObjectTypeInterface* op = dynamic_cast<ObjectTypeInterface*>(parents[i].first);
    if (op)
    {
      int offset = op->inherits(other);
      if (offset != -1)
      {
        qiLogDebug() << "Inheritance offsets " << parents[i].second
         << " " << offset;
        return parents[i].second + offset;
      }
    }
    qiLogDebug() << parents[i].first->infoString() << " does not match " << other->infoString()
    <<" " << ((op != 0) == (dynamic_cast<ObjectTypeInterface*>(other) != 0));
  }
  return -1;
}

namespace detail
{
  ProxyGeneratorMap& proxyGeneratorMap()
  {
    static ProxyGeneratorMap* map = 0;
    if (!map)
      map = new ProxyGeneratorMap();
    return *map;
  }
}

}

#ifdef _MSC_VER
#  pragma warning( pop )
#endif
