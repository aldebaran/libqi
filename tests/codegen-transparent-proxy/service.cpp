
#include "task_interface.hpp"
#include "task_proxy.hpp"
#include "taskservice_interface.hpp"

#include <qitype/objectfactory.hpp>


class TaskService;

// service-side task implementation
class Task: public ITask, public boost::noncopyable
{
public:
  Task(TaskService& service)
  : _service(service) {}
  int addTwo(int i, int j);
  TaskService& _service;
};

QI_IMPLEMENT_ITask(Task);

class TaskService: public ITaskService
{
public:
  ITaskPtr make(int v)
  {
    ITaskPtr result = ITaskPtr(new Task(*this));
    result->value.set(v);
    _tasks.push_back(result);
    return result;
  }
  int accept(ITaskPtr task)
  {
    _tasks.push_back(task);
    return _tasks.size();
  }
  int add(int v)
  {
    int res = 0;
    std::vector<ITaskPtr> t = _tasks;
    for (unsigned i=0; i<t.size(); ++i)
      res += t[i]->addTwo(v, value.get());
    return res;
  }
  void clear()
  {
    _tasks.clear();
  }
private:
  std::vector<ITaskPtr> _tasks;
};


int Task::addTwo(int i, int j)
{
  int res = i+j + value.get() + _service.value.get();
  onAdd(res);
  return res;
}
//QI_IMPLEMENT_ITask(Task);
QI_IMPLEMENT_ITaskService(TaskService);
QI_REGISTER_OBJECT_FACTORY_BUILDER(TaskService);

