/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include <cstring>

#include <qi/assert.hpp>
#include <qi/signature.hpp>
#include <qi/type/typeinterface.hpp>
#include <qi/jsoncodec.hpp>
#include <boost/make_shared.hpp>
#include "signatureconvertor.hpp"

qiLogCategory("qitype.signature");

namespace qi {

  static std::string makeTupleAnnotation(const std::string& name, const std::vector<std::string>& annotations) {
    std::string res;

    if (name.empty() && annotations.empty())
      return res;
    res += '<';
    res += name;
    for (unsigned i = 0; i < annotations.size(); ++i)
      res += ',' + annotations[i];
    res += '>';
    return res;
  }

  qi::Signature makeListSignature(const qi::Signature &element) {
    std::string res;
    res += (char)Signature::Type_List;
    res += element.toString();
    res += (char)Signature::Type_List_End;
    return qi::Signature(res);
  }

  qi::Signature makeVarArgsSignature(const qi::Signature &element) {
    std::string res;
    res += (char)Signature::Type_VarArgs;
    res += element.toString();
    return qi::Signature(res);
  }

  qi::Signature makeKwArgsSignature(const qi::Signature &element) {
    std::string res;
    res += (char)Signature::Type_KwArgs;
    res += element.toString();
    return qi::Signature(res);
  }

  qi::Signature makeTupleSignature(const qi::Signature &element) {
    std::string res;
    res += (char)Signature::Type_Tuple;
    res += element.toString();
    res += (char)Signature::Type_Tuple_End;
    return qi::Signature(res);
  }

  qi::Signature makeMapSignature(const qi::Signature &key, const qi::Signature &value) {
    std::string res;
    res += (char)Signature::Type_Map;
    res += key.toString();
    res += value.toString();
    res += (char)Signature::Type_Map_End;
    return qi::Signature(res);
  }

  qi::Signature makeTupleSignature(const std::vector<qi::AnyReference>& vgv, bool resolve, const std::string &name, const std::vector<std::string>& names)
  {
    std::string res;
    res += (char)Signature::Type_Tuple;
    for (unsigned int i = 0; i < vgv.size(); ++i)
      res += vgv[i].signature(resolve).toString();
    res += (char)Signature::Type_Tuple_End;
    if (names.size() == vgv.size())
      res += makeTupleAnnotation(name, names);
    return qi::Signature(res);
  }

  qi::Signature makeTupleSignature(const std::vector<TypeInterface*>& vt, const std::string &name, const std::vector<std::string>& names)
  {
    std::string res;
    res += (char)Signature::Type_Tuple;
    for (unsigned int i = 0; i < vt.size(); ++i)
      res += vt[i]->signature().toString();
    res += (char)Signature::Type_Tuple_End;
    if (names.size() == vt.size())
      res += makeTupleAnnotation(name, names);
    return qi::Signature(res);
  }

  qi::Signature makeOptionalSignature(const qi::Signature& value)
  {
    const std::string res = [&]() {
      std::stringstream s;
      s << static_cast<char>(Signature::Type_Optional) << value.toString();
      return s.str();
    }();
    return qi::Signature(res);
  }


  float Signature::isConvertibleTo(const qi::Signature& b) const
  {
    /* The returned float is just a basic heuristic, it does not handle:
     * - comparison between integral types
     * - Weaker error score for deeper (in containers) struct.
     */
    float error = 0.f;
    float childErr = 1.0f;
    const auto calculateFactor = [&] {
      return childErr * (1.f - error / 100.f);
    };
    static const char numeric[] = "bcCwWiIlLfd";
    static const char integral[] = "bcCwWiIlL";
    static const char floating[] = "fd";
    static const char container[] = "[{(";

    Signature::Type s = type();
    Signature::Type d = b.type();

    //varargs are just vector, handle them that way
    if (s == Type_VarArgs)
      s = Type_List;
    if (d == Type_VarArgs)
      d = Type_List;
    if (d == Type_Void)
      return calculateFactor();
    if (d == Type_Unknown)
    {
      // We cannot anwser the question for unknown types. So let it pass
      // and the conversion code will decide.
      // Type_Unknown is not serializable anyway.
      if (s != Type_Unknown)
        error += 10.f; // Weird but can happen with object pointers
      return calculateFactor();
    }

    if (d == Type_Dynamic || // Dynamic can convert to whatever
        s == Type_None) // None means parent is empty container
    {
      error += 5.f; // big malus for dynamic
      return calculateFactor();
    }

    // Source is convertible to an optional if source's type is convertible to the destination
    // optional value type or if source's type is void. For instance, int is convertible to
    // optional<int>, but also int is convertible to optional<dynamic>
    if (d == Type_Optional)
    {
      // If source is also an optional then we are performing a optional to optional conversion.
      // By design this is allowed if source value type is convertible to dest value type.
      if (s == Type_Optional)
        return children()[0].isConvertibleTo(b.children()[0]);

      // Converting void to optional means constructing an empty optional. This design choice was
      // made to simplify conversion from language bindings where types are not explicit (like
      // python).
      else if (s == Type_Void)
        return calculateFactor();

      return isConvertibleTo(b.children()[0]);
    }
    else if (s == Type_Optional)
    {
      // The case where dest is dynamic is already handled above, and is the same for optionals:
      // converting optionals to dynamic is allowed, but converting optionals to anything else is
      // not.
      return 0.f;
    }

    // Main switch on source sig
    if (strchr(numeric, s))
    { // Numeric
      if (!strchr(numeric, d))
        return 0.f;
      // Distance heuristic
      if (strchr(integral, s) && strchr(floating, d))
        error += 2.f; // integral->float
      if (strchr(floating, s) && strchr(integral, d))
        error += 3.f; // float->integral
      if (s!= 'b' && d == 'b')
        error += 4.f; // ->bool
    }
    else if (strchr(container, s))
    { // Container, list or map
      if (d != s)
        return 0.f; // Must be same container
      if (children().size() != b.children().size())
      {
        if (s != Type_Tuple)
          return 0.f;
        // Special case for same-named tuples that might be compatible
        std::string aSrc = annotation();
        std::string aDst = b.annotation();
        // This mode is recommended only for tests where it is more
        // conveniant to have differently named structs
        static bool requireSameName = qi::os::getenv("QI_IGNORE_STRUCT_NAME").empty();
        if (!requireSameName)
          return (aSrc.empty() || aDst.empty()) ? 0.f : 0.1f;

        size_t pSrc = aSrc.find_first_of(",");
        size_t pDst = aDst.find_first_of(",");
        if (pSrc == pDst && pSrc != aSrc.npos && !memcmp(aSrc.data(), aDst.data(), pSrc))
          return 0.1f;

        return 0.f;
      }
      SignatureVector::const_iterator its;
      SignatureVector::const_iterator itd;
      itd = b.children().begin();
      for (its = children().begin(); its != children().end(); ++its, ++itd) {
        float childRes = its->isConvertibleTo(*itd);
        if (childRes == 0.f)
          return 0.f; // Just check subtype compatibility
        // we got this far, if there is an error in child, make it lower
        // [s] -> m should have a greater convertibility than [s] -> [m]
        childErr *= 1.0f - (1.0f - childRes) * 0.95f;
      }
      QI_ASSERT(its==children().end() && itd==b.children().end()); // we already exited on size mismatch
    }
    else if (d != s)
      return 0.f;
    return calculateFactor();
  }

  Signature Signature::fromType(Signature::Type t)
  {
    char res[2] = { (char)t, 0};
    return Signature(res);
  }

  //find a open char, starting from end, finishing at start
  static size_t _find_begin(const std::string &str, size_t start, char open, char close)
  {
    int count = 0; // start at 0

    if (str.size() == 0) {
      qiLogDebug() << "_find_begin error: given signature is empty.";
      return std::string::npos;
    }

    size_t i = str.size() - 1;

    if (str[i] != close) {
      qiLogDebug() << "_find_begin error:" << str << "(" << i << ") is not " << close;
      return std::string::npos;
    }

    while (i > start)
    { // update count with current before testing
      if (str[i] == close)
        ++count;
      else if (str[i] == open)
        --count;
      if (!count)
        return i;
      --i;
    }
    return std::string::npos;
  }

  static size_t _find_end(const std::string &str, size_t index, char copen, char close)
  {
    int         count  = 0;

    if (str[index] != copen) {
      qiLogDebug() << "_find_end error:" << str << "(" << index << ") is not " << copen;
      return std::string::npos;
    }

    while ((index < str.size()))
    {
      if (str[index] == copen)
        count++;
      if (str[index] == close)
        count--;
      if (count == 0)
        return index;
      index++;
    }
    return std::string::npos;
  }


  class SignaturePrivate {
  public:
    void parseChildren(const std::string &signature, size_t index);
    void eatChildren(const std::string &signature, size_t idxStart, size_t expectedEnd, int elementCount);
    void init(const std::string &signature, size_t begin, size_t end);

    std::string            _signature;
    std::vector<Signature> _children;
  };

  static size_t findNext(const std::string &signature, size_t index) {

    if (index >= signature.size())
      return std::string::npos;

    //verify that the current signature is correct
    switch(static_cast<qi::Signature::Type>(signature[index]))
    {
      case qi::Signature::Type_Void:
      case qi::Signature::Type_Bool:
      case qi::Signature::Type_Int8:
      case qi::Signature::Type_UInt8:
      case qi::Signature::Type_Int16:
      case qi::Signature::Type_UInt16:
      case qi::Signature::Type_Int32:
      case qi::Signature::Type_UInt32:
      case qi::Signature::Type_Int64:
      case qi::Signature::Type_UInt64:
      case qi::Signature::Type_Float:
      case qi::Signature::Type_Double:
      case qi::Signature::Type_String:
      case qi::Signature::Type_Dynamic:
      case qi::Signature::Type_Raw:
      case qi::Signature::Type_Object:
      case qi::Signature::Type_Unknown:
      case qi::Signature::Type_None:
        index++;
        break;
      case qi::Signature::Type_List:
        index = _find_end(signature, index, '[', ']');
        if (index == std::string::npos)
          return std::string::npos;
        index++;
        break;
      case qi::Signature::Type_VarArgs:
      case qi::Signature::Type_KwArgs:
      case qi::Signature::Type_Optional:
        index++;
        index = findNext(signature, index);
        if (index == std::string::npos)
          return std::string::npos;
        break;
      case qi::Signature::Type_Map:
        index = _find_end(signature, index, '{', '}');
        if (index == std::string::npos)
          return std::string::npos;
        index++;
        break;
      case qi::Signature::Type_Tuple:
        index = _find_end(signature, index, '(', ')');
        if (index == std::string::npos)
          return std::string::npos;
        index++;
        break;
      default:
        qiLogDebug() << "Signature element is invalid: '" << signature << "'";
        return std::string::npos;
    }
    // handle annotation
    if (index < signature.size() && signature[index] == '<')
    {
      index = _find_end(signature, index, '<', '>');
      if (index == std::string::npos)
        return std::string::npos;
      index++;
    }
    return index;
  }


  void SignaturePrivate::eatChildren(const std::string &signature, size_t idxStart, size_t expectedEnd, int elementCount = -1) {
    int i = 0;

    //empty tuple, move on!
    if (elementCount == -1 || elementCount == 0) {
      if (idxStart == expectedEnd)
        return;
    }
    //iterate over each subelement.
    while (true) {
      size_t idxStop = findNext(signature, idxStart);
      _children.push_back(qi::Signature(signature, idxStart, idxStop));
      ++i;
      //ouch too far..
      if (idxStop > expectedEnd) {
        std::stringstream ss;
        ss << "Bad element for signature '" << signature << "' at pos:" << idxStart << " (gone too far)";
        throw std::runtime_error(ss.str());
      }
      if (elementCount == -1) {
        //check if it's the last item.
        if (idxStop == expectedEnd)
          return;
      } else if (elementCount == i) {
        //we have the good number of item, are we at the good position?
        if (idxStop != expectedEnd) {
          std::stringstream ss;
          ss << "Bad element for signature '" << signature << "' at pos:" << idxStart;
          throw std::runtime_error(ss.str());
        }
        return;
      }
      if (idxStart == idxStop)
        throw std::runtime_error("Infinite loop detected..");
      idxStart = idxStop;
    }
  }

  void SignaturePrivate::parseChildren(const std::string &signature, size_t index) {
    if (index >= signature.size())
      throw std::runtime_error("Invalid index");

    //verify that the current signature is correct
    switch(static_cast<qi::Signature::Type>(signature[index]))
    {
      case qi::Signature::Type_Void:
      case qi::Signature::Type_Bool:
      case qi::Signature::Type_Int8:
      case qi::Signature::Type_UInt8:
      case qi::Signature::Type_Int16:
      case qi::Signature::Type_UInt16:
      case qi::Signature::Type_Int32:
      case qi::Signature::Type_UInt32:
      case qi::Signature::Type_Int64:
      case qi::Signature::Type_UInt64:
      case qi::Signature::Type_Float:
      case qi::Signature::Type_Double:
      case qi::Signature::Type_String:
      case qi::Signature::Type_Dynamic:
      case qi::Signature::Type_Raw:
      case qi::Signature::Type_Object:
      case qi::Signature::Type_Unknown:
      case qi::Signature::Type_None:
        return;
      case qi::Signature::Type_List: {
        const auto index_should_stop = _find_end(signature, index, '[', ']');
        eatChildren(signature, index + 1, index_should_stop, 1);
        break;
      }
      case qi::Signature::Type_KwArgs:
      case qi::Signature::Type_VarArgs:
      case qi::Signature::Type_Optional:
      {
        const auto index_should_stop = findNext(signature, index);
        eatChildren(signature, index + 1, index_should_stop, 1);
        break;
      }
      case qi::Signature::Type_Map: {
        const auto index_should_stop = _find_end(signature, index, '{', '}');
        eatChildren(signature, index + 1, index_should_stop, 2);
        break;
      }
      case qi::Signature::Type_Tuple: {
        const auto index_should_stop = _find_end(signature, index, '(', ')');
        eatChildren(signature, index + 1, index_should_stop);
        break;
      }
      default: {
        std::stringstream ss;
        ss << "Signature element is invalid: '" << signature << "'";
        throw std::runtime_error(ss.str());
      }
    }
  }

  //empty signature are invalid
  void SignaturePrivate::init(const std::string &signature, size_t begin, size_t end) {
    size_t index = findNext(signature, begin);
    if (index != end) {
      throw std::runtime_error("Invalid signature");
    }
    parseChildren(signature, begin);
    _signature.assign(signature, begin, end - begin);
  }

  Signature::Signature()
    : _p(boost::make_shared<SignaturePrivate>())
  {
  }

  Signature::Signature(const char *signature)
    : _p(boost::make_shared<SignaturePrivate>())
  {
    _p->_signature.assign(signature);
    _p->init(_p->_signature, 0, _p->_signature.size());
  }


  Signature::Signature(const std::string &signature)
    : _p(boost::make_shared<SignaturePrivate>())
  {
    _p->init(signature, 0, signature.size());
  }

  Signature::Signature(const std::string &signature, size_t begin, size_t end)
    : _p(boost::make_shared<SignaturePrivate>())
  {
    _p->init(signature, begin, end);
  }

  bool Signature::isValid() const {
    return type() != qi::Signature::Type_None;
  }

  const std::string& Signature::toString() const {
    return _p->_signature;
  }

  bool Signature::hasChildren() const {
    return children().size() != 0;
  }

  const SignatureVector& Signature::children() const {
    return _p->_children;
  }

  std::string Signature::annotation()const {
    if (_p->_signature.empty())
      return std::string();
    if (_p->_signature[_p->_signature.length()-1] != '>')
      return std::string();
    size_t begin = _find_begin(_p->_signature, 0, '<', '>');
    if (begin != std::string::npos)
      return std::string(_p->_signature.substr(begin + 1, _p->_signature.size() - begin - 2));
    return std::string();
  }

  std::string Signature::toPrettySignature() const {
    if (!isValid())
      return std::string("Invalid");
    SignatureConvertor sc(this);
    return sc.signature();
  }

  Signature::Type Signature::type() const {
    if (_p->_signature.empty())
      return Type_None;
    return static_cast<Type>(_p->_signature[0]);
  }

  //TODO: There is many Room for improvement HERE
  std::vector<std::string> signatureSplit(const std::string &fullSignature) {
    std::vector<std::string> ret;
    std::string retSig;
    std::string parSig;
    std::string funcName;

    size_t idx1 = fullSignature.find("::");
    if (idx1 != fullSignature.npos) {
      // If :: are given, expect parameter tuple.
      if (fullSignature.find("(") == fullSignature.npos || fullSignature.find(")") == fullSignature.npos)
        throw std::runtime_error("Signature " + fullSignature + " is not valid");

      funcName = fullSignature.substr(0, idx1);
      // we should have a valid signature
      qi::Signature parent("(" + fullSignature.substr(idx1+2) + ")");
      const qi::SignatureVector& childs = parent.children();

      // Expect valid signatures.
      if (fullSignature.substr(idx1+2) == "" || parent.isValid() == false)
        throw std::runtime_error("Signature " + fullSignature + " is not valid");

      if (childs.size() == 2)
      {
        retSig = childs.at(0).toString();
        parSig = childs.at(1).toString();
      }
      else if (childs.size() == 1)
        parSig = childs.at(0).toString();
    } else {
      funcName = fullSignature;
    }
    ret.push_back(retSig);
    ret.push_back(funcName);
    ret.push_back(parSig);
    return ret;
  }

  AnyValue Signature::toData() const
  {
    AnyValueVector res;
    std::string t;
    t += (char)type();
    res.push_back(AnyValue::from(t));
    if (hasChildren()) {
      const SignatureVector& cvec = children();
      SignatureVector::const_iterator it;
      AnyValueVector sub;
      for (it = cvec.begin(); it != cvec.end(); ++it) {
        sub.push_back(it->toData());
      }
      res.push_back(AnyValue::from(sub));
    }
    else
      res.push_back(AnyValue::from(AnyValueVector()));
    res.push_back(AnyValue::from(annotation()));
    return AnyValue::from(res);
  }

  //compare signature without taking annotation into account
  bool operator==(const Signature& lhs, const Signature& rhs)
  {
    if (lhs.type() != rhs.type())
      return false;
    if (lhs.children().size() != rhs.children().size())
      return false;
    SignatureVector::const_iterator it;
    SignatureVector::const_iterator it2;
    it2 = lhs.children().begin();
    for (it = rhs.children().begin(); it != rhs.children().end(); ++it) {
      if (it2 == lhs.children().end()) //not the same number of elements
        return false;
      if (it->type() != it2->type()) //different types
        return false;
      if (*it != *it2) //different children
        return false;
      ++it2;
    }
    return true;
  }

}

char* signature_to_json(const char* sig)
{
  static char* resc = nullptr;
  std::string res;
  try {
    qi::Signature s(sig);
    res = qi::encodeJSON(s.toData());
  } catch (const std::exception& e) {
    qiLogVerbose() << e.what();
    return  nullptr;
  }
  free(resc);
  resc = qi::os::strdup(res.c_str());
  return resc;
}

