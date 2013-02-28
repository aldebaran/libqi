/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include <qitype/signature.hpp>
#include <qitype/functiontype.hpp>
#include <qitype/functiontypefactory.hpp>

qiLogCategory("qitype.functiontype");

namespace qi
{
  GenericValuePtr callManyArgs(FunctionType* type, void* func,
    const std::vector<GenericValuePtr>& args)
  {
    const std::vector<Type*>& target = type->argumentsType();
    if (target.size() != args.size())
    {
      qiLogErrorF("Argument count mismatch, expected %s, got %s",
        target.size(), args.size());
      return GenericValuePtr();
    }
    void** convertedArgs = new void*[args.size()];
     std::vector<GenericValuePtr> toDestroy;
    for (unsigned i=0; i<target.size(); ++i)
    {
      //qiLogDebug() << "argument " << i
      //   << " " << args[i].type->infoString() << ' ' << args[i].value
      //   << " to " << target[i]->infoString();
      if (args[i].type == target[i] || args[i].type->info() == target[i]->info())
        convertedArgs[i] = args[i].value;
      else
      {
        //qiLogDebug() << "needs conversion "
        //<< args[i].type->infoString() << " -> "
        //<< target[i]->infoString();
        std::pair<GenericValuePtr,bool> v = args[i].convert(target[i]);
        if (v.second)
          toDestroy.push_back(v.first);
        convertedArgs[i] = v.first.value;
      }
    }
    void* res = type->call(func, convertedArgs, args.size());
    GenericValuePtr result;
    result.type = type->resultType();
    result.value = res;
    for (unsigned i=0; i<toDestroy.size(); ++i)
      toDestroy[i].destroy();
    delete[] convertedArgs;
    return result;

  }
  GenericValuePtr FunctionType::call(void* func,
    const std::vector<GenericValuePtr>& args)
  {
    unsigned argsSize = args.size();
    if (argsSize > 8)
      return callManyArgs(this, func, args);

    const std::vector<Type*>& target = argumentsType();

    if (target.size() != args.size())
    {
      qiLogErrorF("Argument count mismatch, expected %s, got %s",
        target.size(), args.size());
      return GenericValuePtr();
    }

    void* stackArgs[8];
    void** convertedArgs = stackArgs;
    GenericValuePtr toDestroy[8];
    unsigned int toDestroyPos = 0;
    for (unsigned i=0; i<argsSize; ++i)
    {
      //qiLogDebug() << "argument " << i
      //   << " " << args[i].type->infoString() << ' ' << args[i].value
      //   << " to " << target[i]->infoString();
      if (args[i].type == target[i] || args[i].type->info() == target[i]->info())
        convertedArgs[i] = args[i].value;
      else
      {
        //qiLogDebug() << "needs conversion "
        //<< args[i].type->infoString() << " -> "
        //<< target[i]->infoString();
        std::pair<GenericValuePtr,bool> v = args[i].convert(target[i]);
        if (!v.first.type)
        {
          qiLogError() << "Conversion failure from " << args[i].type->infoString()
          << " to " << target[i]->infoString() <<", aborting call";
          return GenericValuePtr();
        }
        if (v.second)
          toDestroy[toDestroyPos++] = v.first;
        convertedArgs[i] = v.first.value;
      }
    }
    void* res = call(func, convertedArgs, argsSize);
    GenericValuePtr result;
    result.type = resultType();
    result.value = res;
    for (unsigned i=0; i<toDestroyPos; ++i)
      toDestroy[i].destroy();
    return result;
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

  GenericFunctionParameters::GenericFunctionParameters(const std::vector<GenericValuePtr>& args)
  :std::vector<GenericValuePtr>(args)
  {
  }

  GenericFunctionParameters GenericFunctionParameters::copy(bool notFirst) const
  {
    GenericFunctionParameters result(*this);
    for (unsigned i=notFirst?1:0; i<size(); ++i)
      result[i] = result[i].clone();
    return result;
  }

  void GenericFunctionParameters::destroy(bool notFirst)
  {
    for (unsigned i = notFirst ? 1 : 0; i < size(); ++i)
      (*this)[i].destroy();
  }

  GenericFunctionParameters
  GenericFunctionParameters::convert(const Signature& sig) const
  {
    GenericFunctionParameters dst;
    const std::vector<GenericValuePtr>& src = *this;
    if (sig.size() != src.size())
    {
      qiLogError() << "convert: signature/params size mismatch"
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
        qiLogError() <<"convert: unknown type " << *it;
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
//        qiLogError() <<"fromBuffer: unknown type " << *it;
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
      qiLogError() << "Dynamic function called without type information";
      return 0;
    }
    virtual GenericValuePtr call(void* func, const std::vector<GenericValuePtr>& args)
    {
      DynamicFunction* f = (DynamicFunction*)func;
      return (*f)(args);
    }
    _QI_BOUNCE_TYPE_METHODS(DefaultTypeImplMethods<DynamicFunction>);
  };

  FunctionType* dynamicFunctionType()
  {
    static FunctionType* type = 0;
    if (!type)
      type = new DynamicFunctionType();
    return type;
  }

  GenericFunction makeDynamicGenericFunction(DynamicFunction f)
  {
    GenericFunction result;
    result.type = dynamicFunctionType();
    result.value = result.type->clone(result.type->initializeStorage(&f));
    return result;
  }
}
