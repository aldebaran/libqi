#include <iostream>

#include <qimessaging/signature.hpp>

int main()
{
  //pretty print representation of a signature

  //POD
  std::cout << "b                    : " <<  qi::Signature("b").toSTLSignature() << std::endl;
  std::cout << "i                    : " <<  qi::Signature("i").toSTLSignature() << std::endl;
  std::cout << "f                    : " <<  qi::Signature("f").toSTLSignature() << std::endl;
  std::cout << "d                    : " <<  qi::Signature("d").toSTLSignature() << std::endl;
  std::cout << "s                    : " <<  qi::Signature("s").toSTLSignature() << std::endl;

  //STL types
  std::cout << "[i]                  : " <<  qi::Signature("[i]").toSTLSignature() << std::endl;
  std::cout << "{is}                 : " <<  qi::Signature("{is}").toSTLSignature() << std::endl;


  //Qt types
  std::cout << "({is}**)             : " <<  qi::Signature("({is}**)").toQtSignature() << std::endl;
  std::cout << "({is}**)             : " <<  qi::Signature("({is}**)").toQtSignature(true) << std::endl;
  std::cout << "(s{is}**[s][{si}])   : " <<  qi::Signature("(s{is}**[s][{si}])").toQtSignature(true) << std::endl;

  return 0;
}
