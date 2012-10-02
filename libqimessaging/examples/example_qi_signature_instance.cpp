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


  return 0;
}
