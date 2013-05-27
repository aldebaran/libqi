/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include <cstring>

#include <qitype/signature.hpp>
#include <qitype/type.hpp>

qiLogCategory("qi.signature");

namespace qi {

  static std::string makeTupleAnnotation(const std::string& name, const std::vector<std::string>& annotations) {
    std::string res;

    res += '<';
    res += name;
    for (unsigned i = 0; i < annotations.size(); ++i)
      res += ',' + annotations[i];
    res += '>';
    return res;
  }

  qi::Signature makeListSignature(const qi::Signature &element) {
    std::stringstream res;
    res << (char)Signature::Type_List;
    res << element.toString();
    res << (char)Signature::Type_List_End;
    return qi::Signature(res.str());
  }

  qi::Signature makeMapSignature(const qi::Signature &key, const qi::Signature &value) {
    std::stringstream res;
    res << (char)Signature::Type_Map;
    res << key.toString();
    res << value.toString();
    res << (char)Signature::Type_Map_End;
    return qi::Signature(res.str());
  }

  qi::Signature makeTupleSignature(const std::vector<qi::GenericValuePtr>& vgv, bool resolve, const std::string &name, const std::vector<std::string>& names)
  {
    std::stringstream res;
    res << (char)Signature::Type_Tuple;
    for (unsigned int i = 0; i < vgv.size(); ++i)
      res << vgv[i].signature(resolve).toString();
    res << (char)Signature::Type_Tuple_End;
    if (names.size() == vgv.size())
      res << makeTupleAnnotation(name, names);
    return qi::Signature(res.str());
  }

  qi::Signature makeTupleSignature(const std::vector<Type*>& vt, const std::string &name, const std::vector<std::string>& names)
  {
    std::stringstream res;
    res << (char)Signature::Type_Tuple;
    for (unsigned int i = 0; i < vt.size(); ++i)
      res << vt[i]->signature().toString();
    res << (char)Signature::Type_Tuple_End;
    if (names.size() == vt.size())
      res << makeTupleAnnotation(name, names);
    return qi::Signature(res.str());
  }

float qi::Signature::isConvertibleTo(const qi::Signature& b) const
{
  /* The returned float is just a basic heuristic, it does not handle:
   * - comparison between integral types
   * - Weaker error score for deeper (in containers) struct.
  */
  if (size() != b.size())
    return 0;
  int error = 0;
  float childErr = 0.0f;
  static const char numeric[] = "bcCwWiIlLfd";
  static const char integral[] = "bcCwWiIlL";
  static const char floating[] = "fd";
  static const char container[] = "[{(";
  Signature::iterator is = begin();
  Signature::iterator id = b.begin();
  for (;is!=end() && id!= b.end(); ++is,++id)
  {
    Signature::Type s = is.type();
    Signature::Type d = id.type();
    if (d == Type_Unknown)
    {
      // We cannot anwser the question for unknown types. So let it pass
      // and the conversion code will decide.
      // Type_Unknown is not serializable anyway.
      if (s != Type_Unknown)
        error += 10; // Weird but can happen with object pointers
      continue;
    }
    if (d == Type_Dynamic // Dynamic can convert to whatever
      || s == Type_None) // None means parent is empty container
    {
      error += 5; // big malus for dynamic
      continue;
    }
    // Main switch on source sig
    if (strchr(numeric, s))
    { // Numeric
      if (!strchr(numeric, d))
        return 0;
      // Distance heuristic
      if (strchr(integral, s) && strchr(floating, d))
        error += 2; // integral->float
      if (strchr(floating, s) && strchr(integral, d))
        error += 3; // float->integral
      if (s!= 'b' && d == 'b')
        error += 4; // ->bool
    }
    else if (strchr(container, s))
    { // Container, list or map
      if (d != s)
        return 0; // Must be same container
      float childRes = is.children().isConvertibleTo(id.children());
      if (!childRes)
        return 0; // Just check subtype compatibility
      // the lower the error the better
      childErr += 1.0f - childRes*1.01;
    }
    else
      if (d != s)
        return 0;
  }
  assert(is==end() && id==b.end()); // we allready exited on size mismatch
  return 1.0f - childErr - ((float)error) / 100.0f;
}

Signature Signature::fromType(Signature::Type t)
{
  char res[2] = { (char)t, 0};
  return Signature(res);
}

//compare signature without taking annotation into account
bool Signature::operator==(const Signature &rhs) const {
  Signature::iterator it;
  Signature::iterator it2;
  it2 = begin();
  for (it = rhs.begin(); it != rhs.end(); ++it) {
    if (it2 == end()) //not the same number of elements
      return false;
    if (it.type() != it2.type()) //different types
      return false;
    if (it.hasChildren() != it2.hasChildren())
      return false;
    if (it.hasChildren() && it.children() != it2.children()) //different children
      return false;
    ++it2;
  }
  return true;
}

}

QI_EQUIVALENT_STRING_REGISTER(qi::Signature, &qi::Signature::toString);
