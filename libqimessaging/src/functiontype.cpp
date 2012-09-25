/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include <qimessaging/datastream.hpp>
#include <qimessaging/functiontype.hpp>

namespace qi
{
  GenericValue FunctionType::call(void* func,
    const std::vector<GenericValue>& args)
  {
    const std::vector<Type*> target = argumentsType();
    std::vector<GenericValue> toDestroy;
    std::vector<void*> convertedArgs;
    for (unsigned i=0; i<target.size(); ++i)
    {
      qiLogDebug("meta") << "argument " << i
         << " " << args[i].type->infoString() << ' ' << args[i].value
         << " to " << target[i]->infoString();
      if (args[i].type->info() == target[i]->info())
        convertedArgs.push_back(args[i].value);
      else
      {
        qiLogDebug("meta") << "needs conversion "
        << args[i].type->infoString() << " -> "
        << target[i]->infoString();
        std::pair<GenericValue,bool> v = args[i].convert(target[i]);
        if (v.second)
          toDestroy.push_back(v.first);
        convertedArgs.push_back(v.first.value);
      }
    }
    void* res = call(func, convertedArgs);
    GenericValue result;
    result.type = resultType();
    result.value = res;
    for (unsigned i=0; i<toDestroy.size(); ++i)
      toDestroy[i].destroy();
    return result;
  }

  GenericFunction::GenericFunction()
  : type(type), value(value) {}

  GenericFunction::GenericFunction(const GenericValue & v)
  {
    type = dynamic_cast<FunctionType*>(v.type);
    value = type?v.value:0;
  }

  GenericValue GenericFunction::call(const std::vector<GenericValue>& args)
  {
    return type->call(value, args);
  }

  std::string FunctionType::signature() const
  {
    std::string res("(");
    for (unsigned i=0; i<_argumentsType.size(); ++i)
      res += _argumentsType[i]->signature();
    res += ')';
    return res;
  }

  std::string FunctionType::sigreturn() const
  {
    return _resultType->signature();
  }
}
