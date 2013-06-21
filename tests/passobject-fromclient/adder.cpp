#include <qitype/anyobject.hpp>
#include <qitype/objectfactory.hpp>

#include <boost/weak_ptr.hpp>

#include "addtask_proxy.hpp"


qiLogCategory("Adder");

class Adder
{
  public:
  Adder();
  ~Adder() { qiLogVerbose() << "~Adder";}
  int registerTask(AddTaskProxyPtr val);
  int popTask();
  int addAll(int v); // sum add on all tasks
  typedef qi::Property<int> Value;
  qi::Property<int> value;
  private:
  bool onValue(int& storage, const int& newValue);
  std::vector<AddTaskProxyPtr> tasks;

};

QI_REGISTER_OBJECT(Adder, registerTask, popTask, addAll, value);

QI_REGISTER_OBJECT_FACTORY_BUILDER(Adder);


Adder::Adder()
: value(Value::Getter(), boost::bind(&Adder::onValue, this, _1, _2))
{}


bool Adder::onValue(int& storage, const int& newValue)
{
  storage = newValue;
  for (unsigned i=0; i<tasks.size(); ++i)
  {
    tasks[i]->onChange(newValue);
  }
  return true;
}

int Adder::registerTask(AddTaskProxyPtr v)
{
  static int uid = 1;
  tasks.push_back(v);
  v->uid.set(++uid);
  return tasks.size();
}

int Adder::popTask()
{
  AddTaskProxyPtr t = tasks.back();
  tasks.pop_back();
  return t->uid.get();
}

int Adder::addAll(int v)
{
  int res = 0;
  for (unsigned i=0; i<tasks.size(); ++i)
    res += tasks[i]->add(value.get());
  return res;
}
