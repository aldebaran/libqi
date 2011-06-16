#include <iostream>
#include <fstream>

#include <qi/os.hpp>

typedef double (*SinFunctor)(double);


int main(int argc, char *argv[])
{
  void *handle;

  handle = qi::os::dlopen("m");

  if (!handle) {
    std::cerr << "error: could not load libm:" << qi::os::dlerror() << std::endl;
    return 1;
  }

  SinFunctor sinptr;

  sinptr = (SinFunctor)qi::os::dlsym(handle, "sin");

  if (!sinptr) {
    std::cerr << "error: could no found symbol sin" << std::endl;
    return 1;
  }

  std::cout << "sin(42) = " << (*sinptr)(42) << std::endl;

  qi::os::dlclose(handle);
  return 0;
}

