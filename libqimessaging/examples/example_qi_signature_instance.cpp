#include <string>
#include <vector>
#include <map>
#include <iostream>

#include <qimessaging/signature.hpp>

//sample foo function to take the signature from
int foo(int a, int b)
{
  return a + b + 42;
}

int main()
{
  typedef std::map<std::string , std::string> StringMap;
  typedef std::vector<int>                    IntVector;

  StringMap   mymap;
  int         myint;
  std::string mystring;
  IntVector   myvector;

  //take the signature of POD types
  std::cout << "qi::signature< int >                                :" << qi::signatureFromObject::value(myint) << std::endl;

  //take the signature of STL types
  std::cout << "qi::signature< std::string >                        :" << qi::signatureFromObject::value(mystring) << std::endl;
  std::cout << "qi::signature< std::map<std::string, std::string> > :" << qi::signatureFromObject::value(mymap) << std::endl;
  std::cout << "qi::signature< std::vector<int> >                   :" << qi::signatureFromObject::value(myvector) << std::endl;

  //take the signature of a function
  std::cout << "qi::signature< int(int, int) >                      :" << qi::signatureFromObject::value(&foo) << std::endl;

  return 0;
}
