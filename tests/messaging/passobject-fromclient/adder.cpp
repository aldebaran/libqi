#include <qi/anyobject.hpp>
#include <qi/type/objectfactory.hpp>

#include <boost/weak_ptr.hpp>

#include <adder.hpp>


using qi::Object;
using qi::AnyObject;
qiLogCategory("Adder");

class AdderImpl: public Adder
{
  public:
  AdderImpl();
  ~AdderImpl() { qiLogVerbose() << "~Adder";}
  int registerTask(Object<AddTask> val);
  int registerAnyTask(AnyObject v);
  void popTask();
  int addAll(int v); // sum add on all tasks
  using Value = qi::Property<int>;
  private:
  bool onValue(int& storage, const int& newValue);
  std::vector<Object<AddTask> > tasks;

};

QI_REGISTER_IMPLEMENTATION(Adder, AdderImpl);

QI_REGISTER_OBJECT_FACTORY_BUILDER_FOR(Adder, AdderImpl);


AdderImpl::AdderImpl()
{
  value = Value(Value::Getter(), boost::bind(&AdderImpl::onValue, this, _1, _2));
}


bool AdderImpl::onValue(int& storage, const int& newValue)
{
  storage = newValue;
  for (unsigned i=0; i<tasks.size(); ++i)
  {
    tasks[i]->onChange(newValue);
  }
  return true;
}

int AdderImpl::registerTask(Object<AddTask> v)
{
  static int uid = 1;
  tasks.push_back(v);
  v->uid.set(++uid);
  return tasks.size();
}

int AdderImpl::registerAnyTask(AnyObject v)
{
  static int uid = 1;
  tasks.push_back(v);
  tasks.back()->uid.set(++uid);
  return tasks.size();
}

void AdderImpl::popTask()
{
  qiLogVerbose() << "Popping a task...";
  {
    Object<AddTask> t = tasks.back();
    tasks.pop_back();
  }
  qiLogVerbose() << "...popped";
  //return t->uid.get();
}

int AdderImpl::addAll(int v)
{
  int res = 0;
  for (unsigned i=0; i<tasks.size(); ++i)
    res += tasks[i]->add(value.get());
  return res;
}
