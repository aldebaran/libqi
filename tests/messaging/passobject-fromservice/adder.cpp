#include <qi/anyobject.hpp>
#include <qi/type/objectfactory.hpp>

#include <boost/weak_ptr.hpp>

#include <adder.hpp>

qiLogCategory("Adder");
using qi::Object;
using qi::WeakObject;
using qi::AnyObject;

class AdderImpl: public Adder
{
  public:
  AdderImpl();
  ~AdderImpl() { qiLogVerbose() << "~Adder";}
  Object<AddTask> makeTask(int val);
  AnyObject makeAnyTask(int val);
  int nTasks();
  using Value = qi::Property<int>;
  private:
  bool onValue(int& storage, const int& newValue);
  std::vector<WeakObject<AddTask> > tasks;

};

class AddTaskImpl: public AddTask
{
  public:
  AddTaskImpl(Adder& adder, int val)
  : adder(adder)
  , val(val)
  {}
  ~AddTaskImpl() { qiLogVerbose() << "~AddTask";}
  int add(int v) {
    qiLogVerbose("add getting prop");
    int av = adder.value.get();
    qiLogVerbose() << "add " << v <<" " << val << " " << av;
    return v + val + av;
  }
  Adder& adder;
  int val;
};

//QI_TYPE_NOT_CLONABLE(AddTask);
QI_REGISTER_IMPLEMENTATION(AddTask, AddTaskImpl);

//QI_TYPE_NOT_CLONABLE(Adder);
QI_REGISTER_IMPLEMENTATION(Adder, AdderImpl)
QI_REGISTER_OBJECT_FACTORY_BUILDER_FOR(Adder, AdderImpl);

AdderImpl::AdderImpl()
{
  value = Value(Value::Getter(), boost::bind(&AdderImpl::onValue, this, _1, _2));
  qiLogVerbose() << "Prop address: " << &value;
}


int AdderImpl::nTasks()
{
  int res = 0;
  for (unsigned i=0; i<tasks.size(); ++i)
    if (tasks[i].lock())
      ++res;
  return res;
}

bool AdderImpl::onValue(int& storage, const int& newValue)
{
  qiLogVerbose() << "onValue " << newValue << ", updating tasks: " << tasks.size();
  storage = newValue;
  for (unsigned i=0; i<tasks.size(); ++i)
  {
    Object<AddTask> p = tasks[i].lock();
    if (p)
      p->onChange(newValue);
  }
  return true;
}



Object<AddTask> AdderImpl::makeTask(int v)
{
  Object<AddTask> p(new AddTaskImpl(*this, v));
  tasks.push_back(p);
  return p;
}

AnyObject AdderImpl::makeAnyTask(int v)
{
  Object<AddTask> p(new AddTaskImpl(*this, v));
  tasks.push_back(p);
  return p;
}

