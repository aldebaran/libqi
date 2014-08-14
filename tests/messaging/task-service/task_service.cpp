/*
 *  Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 */

#include <boost/make_shared.hpp>

#include <qi/log.hpp>
#include <qi/application.hpp>
#include <qi/anyobject.hpp>
#include <qi/type/dynamicobjectbuilder.hpp>
#include <qi/type/objectfactory.hpp>
#include <qi/session.hpp>
#include <testsession/testsessionpair.hpp>
#include <testsession/testsessionpair.hpp>
#include "task_service.hpp"

qiLogCategory("Task");

TaskImpl::TaskImpl(const std::string& name, TaskGenerator* handler)
  : _handler(handler)
  , _name(name)
  , _param("")
  , _step(0)
{
}

std::string TaskImpl::getName()
{
  return _name;
}

std::string TaskImpl::setParam(const std::string& p)
{
  std::string old = _param;
  _param = p;
  onParamChanged(_param);
  return old;
}

std::string TaskImpl::step(unsigned int arg)
{
  qiLogDebug() << "Step " << this << ' ' << arg;
  std::stringstream result;
  result << _name << ' ' << _param << ' ' << arg << ' ' << ++_step;
  std::string v = result.str();
  _lastResult = v;
  onStep(v);
  return v;
}

std::string TaskImpl::getLastResult()
{
  return _lastResult;
}


qi::Object<Task> TaskGeneratorImpl::newTask(const std::string& name)
{
  qi::Object<Task> task = qi::Object<Task>(new TaskImpl(name, this));
  _tasks.push_back(qi::WeakObject<Task>(task));
  return task;
}
qi::uint64_t TaskGeneratorImpl::taskCount()
{
  unsigned res = 0;
  for (unsigned i=0; i<_tasks.size(); ++i)
    if (_tasks[i].lock())
    ++res;
  return res;
}

void TaskGeneratorImpl::step(unsigned int arg)
{
  for (unsigned i=0; i<_tasks.size(); ++i)
  {
    qi::Object<Task> ptr = _tasks[i].lock();
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

std::vector<qi::Object<Task> > TaskGeneratorImpl::tasks()
{
  std::vector<qi::Object<Task> > result;
  for (unsigned i=0; i<_tasks.size(); ++i)
    if (qi::Object<Task> p = _tasks[i].lock())
      result.push_back(p);
  return result;
}

QI_REGISTER_IMPLEMENTATION(Task, TaskImpl); // declare object
QI_REGISTER_IMPLEMENTATION(TaskGenerator, TaskGeneratorImpl); // declare object
QI_REGISTER_OBJECT_FACTORY_CONSTRUCTOR_FOR(TaskGenerator, TaskGeneratorImpl); // add to .so factory

