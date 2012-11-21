/*
**
** Author(s):
**  - Julien Freche <jfreche@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include "qipython_signal.hpp"

qi_signal::qi_signal()
  : _callbackMap(),
    _sig (qi::Signal<void (std::vector<PyObject*>)>())
{
}

qi_signal::~qi_signal()
{
  disconnectAll();
}

unsigned int qi_signal::connect(PyObject* python_callback)
{
  if (!PyCallable_Check(python_callback))
  {
    qiLogError("qipython.signal") << "Object is not callable";
    return 0;
  }

  Py_XINCREF(python_callback);
  unsigned int id = _sig.connect(boost::bind(&qi_signal::callback, this, python_callback, _1));
  _callbackMap[id] = python_callback;

  return id;
}

bool qi_signal::disconnect(unsigned int link)
{
  std::map<unsigned int, PyObject*>::iterator it = _callbackMap.find(link);

  if (it == _callbackMap.end())
    return false;

  bool res = _sig.disconnect(link);

  Py_XDECREF((*it).second);
  _callbackMap.erase(it);

  return res;
}

bool qi_signal::disconnectAll()
{
  bool res = _sig.disconnectAll();

  for (std::map<unsigned int, PyObject*>::iterator it = _callbackMap.begin();
        it != _callbackMap.end(); it++)
  {
    Py_XDECREF((*it).second);
  }
  _callbackMap.clear();

  return res;
}

void qi_signal::trigger(PyObject* arg0,
                     PyObject* arg1,
                     PyObject* arg2,
                     PyObject* arg3,
                     PyObject* arg4,
                     PyObject* arg5,
                     PyObject* arg6,
                     PyObject* arg7,
                     PyObject* arg8,
                     PyObject* arg9)
{
  std::vector<PyObject*> args;

  args.push_back(arg0);
  args.push_back(arg1);
  args.push_back(arg2);
  args.push_back(arg3);
  args.push_back(arg4);
  args.push_back(arg5);
  args.push_back(arg6);
  args.push_back(arg7);
  args.push_back(arg8);
  args.push_back(arg9);

  _sig(args);
}


void qi_signal::callback(PyObject* callback, const std::vector<PyObject*>& args)
{
  PyObject* ret;
  PyGILState_STATE gstate;
  gstate = PyGILState_Ensure();

  ret = PyObject_CallFunctionObjArgs(callback, args[0], args[1], args[2], args[3], args[4],
                                               args[5], args[6], args[7], args[8], args[9], NULL);
  if (!ret)
  {
    qiLogError("qipython.signal") << "Unable to call python callback";
    PyErr_Print();
    PyErr_Clear();
  }

  Py_XDECREF(ret);

  PyGILState_Release(gstate);
}
