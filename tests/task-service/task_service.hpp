#ifndef TASK_SERVICE_HPP
#define TASK_SERVICE_HPP

class TaskGenerator;

class Task
{
public:
  Task() {}
  Task(const std::string& name, TaskGenerator* handler);
  std::string getName();
  std::string setParam(const std::string& p);
  std::string step(unsigned int arg);
  std::string getLastResult();
  qi::Signal<std::string> onParamChanged;
  qi::Signal<std::string> onStep;
private:
  TaskGenerator* _handler;
  std::string    _name;
  std::string    _param;
  unsigned int   _step;
  std::string    _lastResult;
};


typedef Task TaskImpl;

typedef boost::shared_ptr<Task> TaskPtr;
typedef boost::weak_ptr<Task> TaskWeakPtr;

class TaskGenerator
{
public:
  TaskPtr newTask(const std::string& name);
  void step(unsigned int arg);
  unsigned long taskCount();
  std::vector<TaskPtr> tasks();
private:
  std::vector<TaskWeakPtr> _tasks;
};



typedef TaskGenerator TaskGeneratorImpl;
typedef boost::shared_ptr<TaskGenerator> TaskGeneratorPtr;

#endif
