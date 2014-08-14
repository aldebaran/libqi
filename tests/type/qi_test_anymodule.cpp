#include <qi/anyobject.hpp>
#include <qi/anymodule.hpp>
#include <qi/property.hpp>
#include <qi/type/dynamicobjectbuilder.hpp>

#include "cat.hpp"

void my_void_function(int &a) { a = 44; }
int my_scl_func() { return 45; }
qi::AnyObject my_cat_returner()
{
  qi::Object<Cat> ob(new Cat(15));

  return ob;
}

//TODO
//static const unsigned int MyStaticConstInt = 0xDEADBEEF;
//static const std::string MyStaticConstStr("DEADBEEF");

QI_REGISTER_OBJECT(Cat, meow, meowVolume);


void register_qi_test_module(qi::ModuleBuilder* mb) {
  mb->advertiseFactory<Cat>("Cat");
  mb->advertiseMethod("my_void_function", &my_void_function);
  mb->advertiseMethod("my_scl_func", &my_scl_func);
  mb->advertiseMethod("my_cat_returner", &my_cat_returner);
}

QI_REGISTER_MODULE("qi_test_anymodule", &register_qi_test_module);
