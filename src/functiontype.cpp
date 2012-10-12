/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include <qitype/signature.hpp>
#include <qitype/functiontype.hpp>

namespace qi
{
  GenericValue callManyArgs(FunctionType* type, void* func,
    const std::vector<GenericValue>& args)
  {
    const std::vector<Type*>& target = type->argumentsType();
    void** convertedArgs = new void*[args.size()];
     std::vector<GenericValue> toDestroy;
    for (unsigned i=0; i<target.size(); ++i)
    {
      //qiLogDebug("meta") << "argument " << i
      //   << " " << args[i].type->infoString() << ' ' << args[i].value
      //   << " to " << target[i]->infoString();
      if (args[i].type == target[i] || args[i].type->info() == target[i]->info())
        convertedArgs[i] = args[i].value;
      else
      {
        //qiLogDebug("meta") << "needs conversion "
        //<< args[i].type->infoString() << " -> "
        //<< target[i]->infoString();
        std::pair<GenericValue,bool> v = args[i].convert(target[i]);
        if (v.second)
          toDestroy.push_back(v.first);
        convertedArgs[i] = v.first.value;
      }
    }
    void* res = type->call(func, convertedArgs, args.size());
    GenericValue result;
    result.type = type->resultType();
    result.value = res;
    for (unsigned i=0; i<toDestroy.size(); ++i)
      toDestroy[i].destroy();
    delete[] convertedArgs;
    return result;

  }
  GenericValue FunctionType::call(void* func,
    const std::vector<GenericValue>& args)
  {
    unsigned argsSize = args.size();
    if (argsSize > 8)
      return callManyArgs(this, func, args);

    const std::vector<Type*>& target = argumentsType();

    void* stackArgs[8];
    void** convertedArgs = stackArgs;
    GenericValue toDestroy[8];
    unsigned int toDestroyPos = 0;
    for (unsigned i=0; i<argsSize; ++i)
    {
      //qiLogDebug("meta") << "argument " << i
      //   << " " << args[i].type->infoString() << ' ' << args[i].value
      //   << " to " << target[i]->infoString();
      if (args[i].type == target[i] || args[i].type->info() == target[i]->info())
        convertedArgs[i] = args[i].value;
      else
      {
        //qiLogDebug("meta") << "needs conversion "
        //<< args[i].type->infoString() << " -> "
        //<< target[i]->infoString();
        std::pair<GenericValue,bool> v = args[i].convert(target[i]);
        if (v.second)
          toDestroy[toDestroyPos++] = v.first;
        convertedArgs[i] = v.first.value;
      }
    }
    void* res = call(func, convertedArgs, argsSize);
    GenericValue result;
    result.type = resultType();
    result.value = res;
    for (unsigned i=0; i<toDestroyPos; ++i)
      toDestroy[i].destroy();
    return result;
  }

  GenericFunction::GenericFunction()
  : type(0), value(0) {}

  GenericValue GenericFunction::call(const std::vector<GenericValue>& args)
  {
    return type->call(&value, args);
  }

  std::string CallableType::signature() const
  {
    std::string res("(");
    for (unsigned i=0; i<_argumentsType.size(); ++i)
      res += _argumentsType[i]->signature();
    res += ')';
    return res;
  }

  std::string CallableType::sigreturn() const
  {
    return _resultType->signature();
  }

  GenericFunctionParameters::GenericFunctionParameters()
  {
  }

  GenericFunctionParameters::GenericFunctionParameters(const std::vector<GenericValue>& args)
  :std::vector<GenericValue>(args)
  {
  }

  GenericFunctionParameters GenericFunctionParameters::copy(bool notFirst) const
  {
    GenericFunctionParameters result;
    for (unsigned i=0; i<size(); ++i)
      result.push_back( (!i&&notFirst)? (*this)[i]:(*this)[i].clone());
    return result;
  }

  void GenericFunctionParameters::destroy(bool notFirst)
  {
    for (unsigned i=notFirst?1:0; i<size(); ++i)
      (*this)[i].destroy();
  }

  GenericFunctionParameters
  GenericFunctionParameters::convert(const Signature& sig) const
  {
    GenericFunctionParameters dst;
    const std::vector<GenericValue>& src = *this;
    if (sig.size() != src.size())
    {
      qiLogError("qi.GenericFunctionParameters") << "convert: signature/params size mismatch"
      << sig.toString() << " " << sig.size() << " " << src.size();
      return dst;
    }
    Signature::iterator it = sig.begin();
    int idx = 0;
    for (;it != sig.end(); ++it,++idx)
    {
      Type* compatible = qi::Type::fromSignature(*it);
      if (!compatible)
      {
        qiLogError("qi.GenericFunctionParameters") <<"convert: unknown type " << *it;
        compatible = src[idx].type;
      }
      dst.push_back(src[idx].convertCopy(compatible));
    }
    return dst;
  }

//  GenericFunctionParameters
//  GenericFunctionParameters::fromBuffer(const Signature& sig, const qi::Buffer& buffer)
//  {
//    GenericFunctionParameters result;
//    IDataStream in(buffer);
//    Signature::iterator it = sig.begin();
//    while (it != sig.end())
//    {
//      Type* compatible = qi::Type::fromSignature(*it);
//      if (!compatible)
//      {
//        qiLogError("qi.GenericFunctionParameters") <<"fromBuffer: unknown type " << *it;
//        throw std::runtime_error("Could not construct type for " + *it);
//      }
//      result.push_back(compatible->deserialize(in));
//      ++it;
//    }
//    return result;
//  }

//  Buffer GenericFunctionParameters::toBuffer() const
//  {
//    Buffer buf;
//    ODataStream out(buf);
//    for (unsigned i=0; i<size(); ++i)
//      (*this)[i].serialize(out);
//    return buf;
//  }

  class DynamicFunctionType: public FunctionType
  {
  public:
    virtual void* call(void* func, void** args, unsigned int argc)
    {
      qiLogError("qi.meta") << "Dynamic function called without type information";
      return 0;
    }
    virtual GenericValue call(void* func, const std::vector<GenericValue>& args)
    {
      DynamicFunction* f = (DynamicFunction*)func;
      return (*f)(args);
    }
    _QI_BOUNCE_TYPE_METHODS(DefaultTypeImplMethods<DynamicFunction>);
  };

  GenericFunction makeDynamicGenericFunction(DynamicFunction f)
  {
    static FunctionType* type = 0;
    if (!type)
      type = new DynamicFunctionType();
    GenericFunction result;
    result.type = type;
    *(DynamicFunction*) (void*)&result.value = f;
    return result;
  }
}
