#include <iostream>
#include <qimessaging/signature.hpp>

typedef std::vector<std::string>          StringVect;
typedef std::vector<StringVect>           StringVectVect;
typedef std::map<std::string, StringVect> StringVectMap;
namespace qi
{
  // OLD API compat layer for this test.
  template<typename T> struct signatureFromType
  {
    static std::string value()
    {
      return typeOf<T>()->signature();
    }
    static std::string value(std::string v)
    {
      v+= typeOf<T>()->signature();
      return v;
    }
  };
  struct signatureFromObject
  {
    template<typename T> std::string value(const T& ptr)
    {
      return typeOf(ptr)->signature();
    }
  };
}

void create_a_signature(std::string &raw_signature)
{
  qi::signatureFromType<int>::value(raw_signature);
  qi::signatureFromType<std::string>::value(raw_signature);
  qi::signatureFromType<StringVect>::value(raw_signature);
  qi::signatureFromType<StringVectVect>::value(raw_signature);
  qi::signatureFromType<StringVectMap>::value(raw_signature);
  std::cout << "signature should be: " << "is[s][[s]]{s[s]}" << std::endl;
  std::cout << "actual signature   : " << raw_signature << std::endl;
}

std::string space_i(int space) {
  std::string ret;
  for (int i = 0; i < space; ++i)
    ret += " ";
  return ret;
};

void sig_print(const qi::Signature &sig, int indent = 0) {
  qi::Signature::iterator it;

  for (it = sig.begin(); it != sig.end(); ++it) {
    if (!it.hasChildren()) {
      std::cout << space_i(indent) << "e:(" << it.type() << "): " << *it << std::endl;
    }
    else {
      std::cout << space_i(indent) << "c:(" << it.type() << "): " << *it << std::endl;
      qi::Signature subsig = it.children();
      sig_print(subsig, indent + 1);
    }
  }
};

int main()
{
  std::string raw_signature;

  //get a signature
  create_a_signature(raw_signature);

  qi::Signature           sig(raw_signature.c_str());

  sig_print(sig);

  return 0;
}

/* the output would be:

signature should be: is[s][[s]]{s[s]}
actual signature   : is[s][[s]]{s[s]}
e:(105): i
e:(115): s
c:(91): [s]
 e:(115): s
c:(91): [[s]]
 c:(91): [s]
  e:(115): s
c:(123): {s[s]}
 e:(115): s
 c:(91): [s]
  e:(115): s


*/
