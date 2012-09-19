/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include <cstring>

#include <qimessaging/signature.hpp>

namespace qi {

qi::SignatureStream &operator&(qi::SignatureStream &os, const bool&) {
  os.write(qi::Signature::Type_Bool);
  return os;
}

qi::SignatureStream &operator&(qi::SignatureStream &os, const char&) {
  os.write(qi::Signature::Type_Int8);
  return os;
}

qi::SignatureStream &operator&(qi::SignatureStream &os, const signed char&) {
  os.write(qi::Signature::Type_Int8);
  return os;
}

qi::SignatureStream &operator&(qi::SignatureStream &os, const unsigned char&) {
  os.write(qi::Signature::Type_UInt8);
  return os;
}

qi::SignatureStream &operator&(qi::SignatureStream &os, const short&) {
  os.write(qi::Signature::Type_Int16);
  return os;
}

qi::SignatureStream &operator&(qi::SignatureStream &os, const unsigned short&) {
  os.write(qi::Signature::Type_UInt16);
  return os;
}

qi::SignatureStream &operator&(qi::SignatureStream &os, const int&) {
  os.write(qi::Signature::Type_Int32);
  return os;
}

qi::SignatureStream &operator&(qi::SignatureStream &os, const unsigned int&) {
  os.write(qi::Signature::Type_UInt32);
  return os;
}

qi::SignatureStream &operator&(qi::SignatureStream &os, const long&) {
  os.write(qi::Signature::Type_Int64);
  return os;
}

qi::SignatureStream &operator&(qi::SignatureStream &os, const unsigned long&) {
  os.write(qi::Signature::Type_UInt64);
  return os;
}

qi::SignatureStream &operator&(qi::SignatureStream &os, const long long&) {
  os.write(qi::Signature::Type_Int64);
  return os;
}

qi::SignatureStream &operator&(qi::SignatureStream &os, const unsigned long long&) {
  os.write(qi::Signature::Type_UInt64);
  return os;
}

qi::SignatureStream &operator&(qi::SignatureStream &os, const float&) {
  os.write(qi::Signature::Type_Float);
  return os;
}

qi::SignatureStream &operator&(qi::SignatureStream &os, const double&) {
  os.write(qi::Signature::Type_Double);
  return os;
}

qi::SignatureStream &operator&(qi::SignatureStream &os, const std::string&) {
  os.write(qi::Signature::Type_String);
  return os;
}

qi::SignatureStream &operator&(qi::SignatureStream &os, const char*) {
  os.write(qi::Signature::Type_String);
  return os;
}

qi::SignatureStream &operator&(qi::SignatureStream &os, char*) {
  os.write(qi::Signature::Type_String);
  return os;
}

bool qi::Signature::isConvertibleTo(const qi::Signature& b) const
{
  if (size() != b.size())
    return false;
  static const char numeric[] = "bcCwWiIlLfd";
  static const char container[] = "[{";
  Signature::iterator s = begin();
  Signature::iterator d = b.begin();
  for (;s!=end() && d!= b.end(); ++s,++d)
  {
    if (d.type() == Type_Dynamic)
      continue; // Dynamic can convert to whatever
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
