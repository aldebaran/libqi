#include <qimessaging/datastream.hpp>
#include <qimessaging/function_type.hpp>

namespace qi
{
  inline GenericValue FunctionType::call(void* func,
    const std::vector<GenericValue>& args)
  {
    const std::vector<Type*> target = argumentsType();
    std::vector<GenericValue> toDestroy;
    std::vector<void*> convertedArgs;
    for (unsigned i=0; i<target.size(); ++i)
    {
      qiLogDebug("meta") << "argument " << i << " " << args[i].value;
      if (args[i].type->info() == target[i]->info())
        convertedArgs.push_back(args[i].value);
      else
      {
        qiLogDebug("meta") << "needs conversion "
        << args[i].type->infoString() << " -> "
        << target[i]->infoString();
        GenericValue v = args[i].convert(*target[i]);
        toDestroy.push_back(v);
        convertedArgs.push_back(v.value);
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
