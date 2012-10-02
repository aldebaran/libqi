/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include <qimessaging/metamethod.hpp>
#include <qimessaging/datastream.hpp>

namespace qi {


  MetaFunctionResult callFunction(GenericFunction val,
    const MetaFunctionParameters& parameters)
  {
    if (parameters.getMode() == MetaFunctionParameters::Mode_GenericValue)
    {
      // Let call() handle conversion
      GenericValue res = val.call(parameters.getValues());
      return MetaFunctionResult(res);
    }
    else
    {
      IDataStream in(parameters.getBuffer());
      const std::vector<Type*>& argTypes = val.type->argumentsType();
      std::vector<GenericValue> args;
      for (unsigned i=0; i<argTypes.size(); ++i)
      {
        GenericValue v = argTypes[i]->deserialize(in);
        args.push_back(v);
      }
      GenericValue res = val.call(args);
      for (unsigned i=0; i<args.size(); ++i)
        args[i].destroy();
      return MetaFunctionResult(res);
    }
  }

  MetaFunctionResult callMethod(GenericMethod val,
    GenericValue instance, const MetaFunctionParameters& parameters)
  {
    if (parameters.getMode() == MetaFunctionParameters::Mode_GenericValue)
    {
      return MetaFunctionResult(val.call(instance, parameters.getValues()));
    }
    else
    {
      IDataStream in(parameters.getBuffer());
      const std::vector<Type*>& argTypes = val.type->argumentsType();
      std::vector<GenericValue> args;
      for (unsigned i=1; i<argTypes.size(); ++i)
      {
        GenericValue v = argTypes[i]->deserialize(in);
        args.push_back(v);
      }
      GenericValue res = val.call(instance, args);
      for (unsigned i=0; i<args.size(); ++i)
        args[i].destroy();
      return MetaFunctionResult(res);
    }
  }

  MetaCallable makeCallable(GenericFunction value)
  {
    return boost::bind(&callFunction, value, _1);
  }

  MetaMethod::MetaMethod(unsigned int uid,
    const std::string& sigret,
    const std::string& signature)
  : _uid(uid)
  , _signature(signature)
  , _sigreturn(sigret)
  {}

  std::string MetaMethod::signature() const
  {
    return _signature;
  }

  std::string MetaMethod::sigreturn() const
  {
    return _sigreturn;
  }

  unsigned int       MetaMethod::uid() const
  {
    return _uid;
  }

};
