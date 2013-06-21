#include <qitype/anyobject.hpp>
#include <qitype/objectfactory.hpp>

#include <boost/weak_ptr.hpp>

class AddTask;
typedef boost::shared_ptr<AddTask> AddTaskPtr;
typedef boost::weak_ptr<AddTask> AddTaskWeakPtr;

qiLogCategory("Adder");

class Adder
{
  public:
  Adder();
  ~Adder() { qiLogVerbose() << "~Adder";}
  AddTaskPtr makeTask(int val);
  typedef qi::Property<int> Value;
  qi::Property<int> value;
  private:
  bool onValue(int& storage, const int& newValue);
  std::vector<AddTaskWeakPtr> tasks;

};

class AddTask
{
  public:
  AddTask(Adder& adder, int val)
  : adder(adder)
  , val(val)
  {}
  ~AddTask() { qiLogVerbose() << "~AddTask";}
  int add(int v) {
    qiLogVerbose("add getting prop");
    int av = adder.value.get();
    qiLogVerbose() << "add " << v <<" " << val << " " << av;
    return v + val + av;
  }
  qi::Signal<int> onChange;
  Adder& adder;
  int val;
};

QI_REGISTER_OBJECT(AddTask, add, onChange);
QI_TYPE_NOT_CLONABLE(AddTask);

QI_REGISTER_OBJECT(Adder, makeTask, value);
QI_REGISTER_OBJECT_FACTORY_BUILDER(Adder);


Adder::Adder()
: value(Value::Getter(), boost::bind(&Adder::onValue, this, _1, _2))
{}


bool Adder::onValue(int& storage, const int& newValue)
{
  storage = newValue;
  for (unsigned i=0; i<tasks.size(); ++i)
  {
    AddTaskPtr p = tasks[i].lock();
    if (p)
      p->onChange(newValue);
  }
  return true;
}



AddTaskPtr Adder::makeTask(int v)
{
  AddTaskPtr p(new AddTask(*this, v));
  tasks.push_back(p);
  return p;
}

