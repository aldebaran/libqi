#include <qitype/objectfactory.hpp>
#include <qitype/objecttypebuilder.hpp>

class TestObject
{
public:
  int getv() { return v;}
  void setv(int i) { v = i;}
  int add(int i) { int res =  i+v; lastRes = res; return res;}
  int lastAdd() { return lastRes;}
  int v;
  int lastRes;
};
QI_REGISTER_OBJECT(TestObject, add, getv, setv, v, lastAdd);

QI_REGISTER_OBJECT_FACTORY_CONSTRUCTOR(TestObject);
