/*
 *  Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 */


#include <boost/make_shared.hpp>

#include <qi/qi.hpp>
#include <qi/application.hpp>
#include <qitype/genericobject.hpp>
#include <qitype/genericobjectbuilder.hpp>
#include <qimessaging/session.hpp>
#include <testsession/testsessionpair.hpp>
#include <qimessaging/servicedirectory.hpp>
#include <testsession/testsessionpair.hpp>

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
  qi::Signal<void(std::string)> onParamChanged;
  qi::Signal<void(std::string)> onStep;
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

Task::Task(const std::string& name, TaskGenerator* handler)
  : _handler(handler)
  , _name(name)
  , _param("")
  , _step(0)
{
}

std::string Task::getName()
{
  return _name;
}

std::string Task::setParam(const std::string& p)
{
  std::string old = _param;
  _param = p;
  onParamChanged(_param);
  return old;
}

std::string Task::step(unsigned int arg)
{
  std::stringstream result;
  result << _name << ' ' << _param << ' ' << arg << ' ' << ++_step;
  std::string v = result.str();
  _lastResult = v;
  onStep(v);
  return v;
}

std::string Task::getLastResult()
{
  return _lastResult;
}


TaskPtr TaskGenerator::newTask(const std::string& name)
{
  TaskPtr task = boost::make_shared<Task>(name, this);
  _tasks.push_back(TaskWeakPtr(task));
  return task;
}
unsigned long TaskGenerator::taskCount()
{
  return _tasks.size();
}

void TaskGenerator::step(unsigned int arg)
{
  for (unsigned i=0; i<_tasks.size(); ++i)
  {
    TaskPtr ptr = _tasks[i].lock();
    if (ptr)
      ptr->step(arg);
    else
    {
      _tasks[i] = _tasks[_tasks.size()-1];
      _tasks.pop_back();
      --i;
    }
  }
}

std::vector<TaskPtr> TaskGenerator::tasks()
{
  std::vector<TaskPtr> result;
  for (unsigned i=0; i<_tasks.size(); ++i)
    if (TaskPtr p = _tasks[i].lock())
      result.push_back(p);
  return result;
}

TaskGeneratorPtr make_TaskGeneratorPtr()
{
  return TaskGeneratorPtr(new TaskGenerator());
}
