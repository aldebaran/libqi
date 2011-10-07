#include <string>
#include <vector>
#include <map>
#include <iostream>

#include <qimessaging/signature.hpp>

int main()
{
  typedef std::map<std::string , std::string> StringMap;

  //take the signature of POD types
  std::cout << "qi::signature<int>                                  :" << qi::signature<int>::value() << std::endl;

  //take the signature of STL types
  std::cout << "qi::signature<std::string>                          :" << qi::signature<std::string>::value() << std::endl;
  std::cout << "qi::signature< std::vector<int> >                   :" << qi::signature< std::vector<int> >::value() << std::endl;
  std::cout << "qi::signature< std::map<std::string, std::string> > :" << qi::signature<StringMap>::value() << std::endl;

  //take the signature of a function
  std::cout << "qi::signature< (void *)(int, int) >                 :" << qi::signature< void(int, int)>::value() << std::endl;

  return 0;
}
