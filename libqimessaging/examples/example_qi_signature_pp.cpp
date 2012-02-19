#include <iostream>

#include <qimessaging/signature.hpp>

int main()
{
  //pretty print representation of a signature

  //POD
  std::cout << "b                    : " <<  qi::signatureToString("b") << std::endl;
  std::cout << "i                    : " <<  qi::signatureToString("i") << std::endl;
  std::cout << "f                    : " <<  qi::signatureToString("f") << std::endl;
  std::cout << "d                    : " <<  qi::signatureToString("d") << std::endl;
  std::cout << "s                    : " <<  qi::signatureToString("s") << std::endl;

  //protobuf currently throws qi::BadSignatureError
  //std::cout << "@ALCompat.ALValue@   : " <<  qi::signatureToString("@ALCompat.ALValue@") << std::endl;

  //STL types
  std::cout << "[i]                  : " <<  qi::signatureToString("[i]") << std::endl;
  std::cout << "{is}                 : " <<  qi::signatureToString("{is}") << std::endl;

  return 0;
}
