#ifndef TASK_HPP
#define TASK_HPP
#include <qi/signal.hpp>
#include <qi/anyobject.hpp>

class Task
{
public:
  virtual std::string getName() = 0;
  virtual std::string setParam(const std::string& p) = 0;
  virtual std::string step(unsigned int arg) = 0;
  virtual std::string getLastResult() = 0;
  qi::Signal<std::string> onParamChanged;
  qi::Signal<std::string> onStep;
};


class TaskGenerator
{
public:
  virtual qi::Object<Task> newTask(const std::string& name) = 0;
  virtual void step(unsigned int arg) = 0;
  virtual qi::uint64_t taskCount() = 0;
  virtual std::vector<qi::Object<Task> > tasks() = 0;
};

#endif
