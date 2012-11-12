/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include <cstring>

#include <qitype/signature.hpp>

namespace qi {

bool qi::Signature::isConvertibleTo(const qi::Signature& b) const
{
  if (size() != b.size())
    return false;
  static const char numeric[] = "bcCwWiIlLfd";
  static const char container[] = "[{(";
  Signature::iterator s = begin();
  Signature::iterator d = b.begin();
  for (;s!=end() && d!= b.end(); ++s,++d)
  {
    if (d.type() == Type_Dynamic // Dynamic can convert to whatever
      || s.type() == Type_None) // None means parent is empty container
      continue;
    // Main switch on source sig
    if (strchr(numeric, s.type()))
    { // Numeric
      if (!strchr(numeric, d.type()))
        return false;
    }
    else if (strchr(container, s.type()))
    { // Container, list or map
      if (d.type() != s.type())
        return false; // Must be same container
      if (!s.children().isConvertibleTo(d.children()))
        return false; // Just check subtype compatibility
    }
    else
      if (d.type() != s.type())
        return false;
  }
  assert(s==end() && d==b.end()); // we allready exited on size mismatch
  return true;
}

Signature Signature::fromType(Signature::Type t)
{
  char res[2] = { (char)t, 0};
  return Signature(res);
}

}
