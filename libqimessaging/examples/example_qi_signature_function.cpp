#include <string>
#include <vector>
#include <map>
#include <iostream>

#include <qi/signature.hpp>

//sample foo function to take the signature from
int foo(int a, int b)
{
  return a + b + 42;
}

int main(int argc, char *argv[])
{
  //wrapper arround signature, to create the signature of a function with it's name
  std::string signature;

  signature = qi::makeFunctionSignature("foo", &foo);
  std::cout << "function signature should be      : foo::i:ii" << std::endl;
  std::cout << "function signature                : " << signature << std::endl;
  std::cout << "Pretty printed function signature : " << qi::signatureToString(signature) << std::endl;
  return 0;
}
