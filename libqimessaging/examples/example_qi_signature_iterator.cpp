#include <iostream>
#include <qimessaging/signature.hpp>

typedef std::vector<std::string>          StringVect;
typedef std::vector<StringVect>           StringVectVect;
typedef std::map<std::string, StringVect> StringVectMap;

void create_a_signature(std::string &raw_signature)
{
  qi::signature<int>::value(raw_signature);
  qi::signature<std::string>::value(raw_signature);
  qi::signature<StringVect>::value(raw_signature);
  qi::signature<StringVectVect>::value(raw_signature);
  qi::signature<StringVectMap>::value(raw_signature);
  std::cout << "signature should be: " << "is[s][[s]]{s[s]}" << std::endl;
  std::cout << "actual signature   : " << raw_signature << std::endl;
}

int main()
{
  std::string raw_signature;

  //get a signature
  create_a_signature(raw_signature);

  qi::Signature           sig(raw_signature);
  qi::Signature::iterator it;

  for (it = sig.begin(); it != sig.end(); ++it) {
    std::cout << "signature: " << it.signature()  << std::endl;
    std::cout << "child_1  : " << it.child_1()    << std::endl;
    std::cout << "child_2  : " << it.child_2()    << std::endl;
  }

  return 0;
}

/* the output would be:

signature should be: is[s][[s]]{s[s]}
actual signature   : is[s][[s]]{s[s]}
signature: i
child_1  :
child_2  :
signature: s
child_1  :
child_2  :
signature: [s]
child_1  : s
child_2  :
signature: [[s]]
child_1  : [s]
child_2  :
signature: {s[s]}
child_1  : s
child_2  : [s]

*/
