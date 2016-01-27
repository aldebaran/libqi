#ifndef TASK_SERVICE_HPP
#define TASK_SERVICE_HPP

#include <taskgenerator.hpp>

class TaskImpl: public Task
{
public:
  TaskImpl() = default;
  TaskImpl(const std::string& name, TaskGenerator* handler);

  std::string getName();
  std::string setParam(const std::string& p);
  std::string step(unsigned int arg);
  std::string getLastResult();
private:
  TaskGenerator* _handler = nullptr;
  std::string    _name;
  std::string    _param;
  unsigned int   _step = 0;
  std::string    _lastResult;
};


class TaskGeneratorImpl: public TaskGenerator
{
public:
  qi::Object<Task> newTask(const std::string& name);
  void step(unsigned int arg);
  qi::uint64_t taskCount();
  std::vector<qi::Object<Task> > tasks();
private:
  std::vector<qi::WeakObject<Task> > _tasks;
};




#endif
