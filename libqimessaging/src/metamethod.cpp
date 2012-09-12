/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <qimessaging/metamethod.hpp>

namespace qi {


  MetaFunctionResult methodvalue_bounce(MethodValue val,
    Value instance,
    const MetaFunctionParameters& parameters)
  {
    if (parameters.getMode() == MetaFunctionParameters::Mode_Value)
    {
      Value res = val.call(instance, parameters.getValues());
      return MetaFunctionResult(res);
    }
    else
    {
      IDataStream in(parameters.getBuffer());
      const std::vector<Type*>& argTypes = val.type->argumentsType();
      std::vector<Value> args;
      // arg0 is the instance
      for (unsigned i=1; i<argTypes.size(); ++i)
      {
        Value v;
        v.type = argTypes[i];
        v.value = v.type->deserialize(in);
        args.push_back(v);
      }
      Value res = val.call(instance, args);
      return MetaFunctionResult(res);
    }
  }

  MetaMethod::MetaMethod(unsigned int uid,
    const std::string& sigret,
    const std::string& signature,
    MetaCallable value)
  :_functor(value)
  , _uid(uid)
  , _signature(signature)
  , _sigreturn(sigret)
  {}

  MetaMethod::MetaMethod(const std::string& name, unsigned int uid, MethodValue value)
  : _uid(uid)
  {
    _signature = name + "::" + value.signature();
    _sigreturn = value.sigreturn();
    _functor = boost::bind(&methodvalue_bounce, value, _1, _2);
  }

  MetaMethod::MetaMethod(unsigned int uid,
    const std::string& sigret,
    const std::string& signature,
    MetaFunction value)
  : _uid(uid)
  , _signature(signature)
  , _sigreturn(sigret)
  {
    _functor = boost::bind(value, _2);
  }

  std::string MetaMethod::signature() const
  {
    return _signature;
  }

  std::string MetaMethod::sigreturn() const
  {
    return _sigreturn;
  }

  const qi::MetaCallable& MetaMethod::functor() const
  {
    return _functor;
  }

  unsigned int       MetaMethod::uid() const
  {
    return _uid;
  }

  qi::ODataStream &operator<<(qi::ODataStream &stream, const MetaMethod &meta) {
    stream << meta.signature()
           << meta.sigreturn()
           << meta.uid();
    return stream;
  }

  qi::IDataStream &operator>>(qi::IDataStream &stream, MetaMethod &meta) {
    unsigned int uid;
    std::string signature, sigret;
    stream >> signature >> sigret >> uid;
    meta = MetaMethod( uid, sigret, signature, MetaCallable());
    return stream;
  }

  qi::SignatureStream &operator&(qi::SignatureStream &stream, const MetaMethod &meta) {
    stream & meta.signature();
    stream & meta.sigreturn();
    stream & meta.uid();
    return stream;
  }

};
