#include <string>
#include <vector>
#include <map>
#include <iostream>

#include <qimessaging/signature.hpp>

namespace qi
{
  // OLD API compat layer for this test.
  template<typename T> struct signatureFromType
  {
    static std::string value()
    {
      return typeOf<T>()->signature();
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

int main()
{
  typedef std::map<std::string , std::string> StringMap;

  //take the signature of POD types
  std::cout << "qi::signature< int >                                :" << qi::signatureFromType<int>::value() << std::endl;

  //take the signature of STL types
  std::cout << "qi::signature< std::string >                        :" << qi::signatureFromType<std::string>::value() << std::endl;
  std::cout << "qi::signature< std::vector<int> >                   :" << qi::signatureFromType< std::vector<int> >::value() << std::endl;
  std::cout << "qi::signature< std::map<std::string, std::string> > :" << qi::signatureFromType<StringMap>::value() << std::endl;

  return 0;
}
