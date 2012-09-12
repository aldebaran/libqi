#include <qimessaging/datastream.hpp>
#include <qimessaging/function_type.hpp>

namespace qi
{
  inline Value FunctionType::call(void* func,
    const std::vector<Value>& args)
  {
    const std::vector<Type*> target = argumentsType();
    std::vector<Value> toDestroy;
    std::vector<void*> convertedArgs;
    for (unsigned i=0; i<target.size(); ++i)
    {
      qiLogDebug("meta") << "argument " << i << " " << args[i].value;
      if (args[i].type->info() == target[i]->info())
        convertedArgs.push_back(args[i].value);
      else
      {
        qiLogDebug("meta") << "needs conversion";
        Value v = args[i].convert(*target[i]);
        toDestroy.push_back(v);
        convertedArgs.push_back(v.value);
      }
    }
    void* res = call(func, convertedArgs);
    Value result;
    result.type = resultType();
    result.value = res;
    for (unsigned i=0; i<toDestroy.size(); ++i)
      toDestroy[i].destroy();
    return result;
  }

  FunctionValue::FunctionValue()
  : type(type), value(value) {}

  FunctionValue::FunctionValue(const Value & v)
  {
    type = dynamic_cast<FunctionType*>(v.type);
    value = type?v.value:0;
  }

  Value FunctionValue::call(const std::vector<Value>& args)
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
