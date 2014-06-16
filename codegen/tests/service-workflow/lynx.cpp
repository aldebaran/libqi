#include "cat.hpp"
#include <qi/anyobject.hpp>
#include <qi/type/objectfactory.hpp>
// Home-brewed cat implementation.

using animals::CatAction;
using animals::Mosquito;

class LynxAction: public animals::CatAction
{
public:
  LynxAction() {}
  LynxAction(const std::string& n) : _name(n) {}
  std::string name() { return _name;}
  std::vector<float> expectedResult()
  { // expect carnage
    std::vector<float> res(_name.length(), 1);
    return res;
  }
  void run()
  {
    qi::os::msleep(_name.length()*100);
  }
  std::string _name;
};

QI_REGISTER_IMPLEMENTATION(animals::CatAction,LynxAction);

class Lynx: public animals::Cat
{
public:
  void meow(int64_t volume);
  bool setTarget(const Mosquito& m);
  qi::Object<CatAction> selectTask(qi::int64_t seed);
  bool canPerform(qi::Object<CatAction> task);
};


bool Lynx::canPerform(qi::Object<CatAction> task)
{
  if (!task)
    throw std::runtime_error("Invalid task");
  std::string name = task->name();
  std::cerr << "name " << name << std::endl;
  // This is a test object, so let's test
  if (name != task.call<std::string>("name").value())
  {
    std::cerr << "##\n" << name << "\n" << task.call<std::string>("name").value() << std::endl;
    throw std::runtime_error("Inconsistency");
  }
  return name.length() >= 4 && name.substr(0, 4) == "lynx";
}

void Lynx::meow(qi::int64_t volume)
{
  if (volume < 10)
    throw std::runtime_error("Seriously? I'm a lynx dammit!");
  boredom.set(boredom.get()-1);
}

bool Lynx::setTarget(const Mosquito& m)
{
  if (m.distance > hunger.get() - boredom.get())
    return false; //BOOO-RING
  cuteness.set(cuteness.get()+1);
  return true;
}

qi::Object<CatAction> Lynx::selectTask(qi::int64_t seed)
{
  if (seed == 1)
  {
    Mosquito m;
    m.distance = 10;
    onTargetDetected(m);
  }
  return new LynxAction("lynxYawn");
}
QI_REGISTER_IMPLEMENTATION(animals::Cat, Lynx);
QI_REGISTER_OBJECT_FACTORY_CONSTRUCTOR_FOR(animals::Lynx, Lynx);

