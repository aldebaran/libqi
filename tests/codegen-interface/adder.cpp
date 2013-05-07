#include <qitype/genericobject.hpp>
#include <qitype/objectfactory.hpp>
#include "adder_interface.hpp"



class Adder0: public IAdder
{
public:
  int addTwo(int a, int b)
  {
    int res = a+b+value.get();
    onAdd(res);
    return res;
  }
};

QI_IMPLEMENT_IAdder(Adder0);

QI_REGISTER_OBJECT_FACTORY_BUILDER(Adder0);


bool clamp_positive(int& tgt, const int& v)
{
  if (v < 0)
    return false;
  tgt = v;
  return true;
}

class Adder1: public IAdder
{
public:
  Adder1()
  : IAdder(
    *new qi::Signal<void(int)>(),
    *new qi::Property<int>(qi::Property<int>::Getter(), clamp_positive)
    )
  {}
  int addTwo(int a, int b)
  {
    int res = a+b+value.get();
    onAdd(res);
    return res;
  }
  int extraStuff() { return 0;} // just to test the macro with args
};


QI_IMPLEMENT_IAdder(Adder1, extraStuff);
QI_REGISTER_OBJECT_FACTORY_BUILDER(Adder1);
