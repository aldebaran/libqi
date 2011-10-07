/**
 *  Copyright (C) 2011 Aldebaran Robotics
 */


#include <iostream>
#include <qi/os.hpp>


int main()
{
  std::string foo = qi::os::getenv("FOO");
  if (foo != "BAR")
  {
    std::cerr << "FOO should be BAR and is: '" << foo << "'" << std::endl;
    return 1;
  }
  return 0;
}

