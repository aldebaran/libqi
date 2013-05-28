/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include <qi/future.hpp>
#include <qitype/signature.hpp>
#include <qitype/functiontype.hpp>
#include <qitype/functiontypefactory.hpp>
#include <qitype/genericobject.hpp>

qiLogCategory("qitype.functiontype");

namespace qi
{
  GenericValuePtr GenericFunction::call(
    GenericValuePtr arg1, const std::vector<GenericValuePtr>& remaining)
  {
    std::vector<GenericValuePtr> args;
    args.reserve(remaining.size()+1);
    args.push_back(arg1);
    args.insert(args.end(), remaining.begin(), remaining.end());
    return call(args);
  }

  GenericValuePtr GenericFunction::call(
    const std::vector<GenericValuePtr>& vargs)
  {
    if (type == dynamicFunctionType())
    {
      DynamicFunction* f = (DynamicFunction*)value;
      if (!transform.dropFirst && !transform.prependValue)
        return (*f)(vargs);
      std::vector<GenericValuePtr> args;
      if (transform.dropFirst && !transform.prependValue)
        args.insert(args.end(), &vargs[1], &vargs[1] + vargs.size()-1);
      else if (transform.dropFirst && transform.prependValue)
      {
        args = vargs;
        args[0].value = transform.boundValue;
      }
      else // prepend && ! drop
        throw std::runtime_error("Cannot prepend argument to dynamic function type");
      return (*f)(args);
    }
    /* We must honor transform, who can have any combination of the following enabled:
    * - drop first arg
    * - prepend an arg
    */
    unsigned deltaCount = (transform.dropFirst? -1:0) + (transform.prependValue?1:0);
    const std::vector<Type*>& target = type->argumentsType();
    unsigned sz = vargs.size();
    const GenericValuePtr* args = sz > 0 ? &vargs[0] : 0;

    if (target.size() != sz + deltaCount)
    {
      throw std::runtime_error(_QI_LOG_FORMAT("Argument count mismatch, expected %s, got %s (transform %s)",
        target.size(), sz, deltaCount));
      return GenericValuePtr();
    }
    if (transform.dropFirst)
    {
      ++args;
      --sz;
    }
    unsigned offset = transform.prependValue? 1:0;
#if QI_HAS_VARIABLE_LENGTH_ARRAY
    GenericValuePtr toDestroy[sz+offset];
    void* convertedArgs[sz+offset];
#else
    GenericValuePtr* toDestroy = new GenericValuePtr[sz+offset];
    void** convertedArgs = new void*[sz+offset];
#endif
    if (transform.prependValue)
      convertedArgs[0] = transform.boundValue;
    unsigned int toDestroyPos = 0;
    for (unsigned i=0; i<sz; ++i)
    {
      //qiLogDebug() << "argument " << i
      //   << " " << args[i].type->infoString() << ' ' << args[i].value
      //   << " to " << target[i]->infoString();
      if (args[i].type == target[i+offset] || args[i].type->info() == target[i+offset]->info())
        convertedArgs[i+offset] = args[i].value;
      else
      {
        //qiLogDebug() << "needs conversion "
        //<< args[i].type->infoString() << " -> "
        //<< target[i]->infoString();
        std::pair<GenericValuePtr,bool> v = args[i].convert(target[i+offset]);
        if (!v.first.type)
        {
          // Try pointer dereference
          if (args[i].kind() == Type::Pointer)
          {
            GenericValuePtr deref = *const_cast<GenericValuePtr&>(args[i]);
            if (deref.type == target[i+offset] || deref.type->info() == target[i+offset]->info())
              v = std::make_pair(deref, false);
            else
              v = deref.convert(target[i+offset]);
          }
          if (!v.first.type)
          {
            throw std::runtime_error(_QI_LOG_FORMAT("Call argument conversion failure from %s to %s (equals: %s)",
              args[i].type->infoString(),
              target[i]->infoString(),
              args[i].type->infoString() == target[i]->infoString()));
            return GenericValuePtr();
          }
        }
        if (v.second)
          toDestroy[toDestroyPos++] = v.first;
        convertedArgs[i+offset] = v.first.value;
      }
    }
    void* res;
    res = type->call(value, convertedArgs, sz+offset);
    GenericValuePtr result;
    result.type = resultType();
    result.value = res;
    for (unsigned i=0; i<toDestroyPos; ++i)
      toDestroy[i].destroy();
#if ! QI_HAS_VARIABLE_LENGTH_ARRAY
    delete[] toDestroy;
    delete[] convertedArgs;
#endif
    return result;
  }

  const GenericFunction& GenericFunction::dropFirstArgument() const
  {
    transform.dropFirst = true;
    return *this;
  }

  const GenericFunction& GenericFunction::prependArgument(void* arg) const
  {
    transform.prependValue = true;
    transform.boundValue = arg;
    return *this;
  }

  const GenericFunction& GenericFunction::replaceFirstArgument(void* arg) const
  {
    transform.dropFirst = true;
    return prependArgument(arg);
  }

  Type* GenericFunction::resultType() const
  {
    return type->resultType();
  }

  std::vector<Type*> GenericFunction::argumentsType() const
  {
    std::vector<Type*> res = type->argumentsType();
    if (transform.dropFirst && transform.prependValue) // optimize that case
      res[0] = typeOf<GenericValue>();
    else if (transform.dropFirst)
    {
      // First argument passed to us will be ignored, so apparent signature
      // has one extra arg of any type.

      // do not access res[1], it might not exist and invalid access might be
      // detected by debug-mode stl
      res.push_back(0);
      memmove(&res[0]+1, &res[0], (res.size()-1)*sizeof(Type*));
      res[0] = typeOf<GenericValue>();
    }
    else if (transform.prependValue)
    {
      // We bind one argument, so it is not present in apparent signature, remove it
      memmove(&res[0], &res[0]+1, (res.size()-1)*sizeof(Type*));
      res.pop_back();
    }
    return res;
  }

  qi::Signature GenericFunction::parametersSignature(bool dropFirst) const
  {
    if (!dropFirst)
      return qi::makeTupleSignature(argumentsType());
    std::vector<Type*> vtype = argumentsType();
    if (vtype.empty())
      throw std::runtime_error("Can't drop the first argument, the argument list is empty");
    //ninja! : drop the first Type*
    memmove(&vtype[0], &vtype[0]+1, (vtype.size()-1)*sizeof(Type*));
    vtype.pop_back();
    return qi::makeTupleSignature(vtype);
  }

  qi::Signature GenericFunction::returnSignature() const
  {
    // Detect if returned type is a qi::Future and return underlying value type.
    TypeTemplate* ft1 = QI_TEMPLATE_TYPE_GET(resultType(), Future);
    TypeTemplate* ft2 = QI_TEMPLATE_TYPE_GET(resultType(), FutureSync);
    TypeTemplate* futureType = ft1 ? ft1 : ft2;
    if (futureType)
      return futureType->templateArgument()->signature();
    else
      return resultType()->signature();
  }

  qi::Signature CallableType::parametersSignature() const
  {
    return qi::makeTupleSignature(_argumentsType);
  }

  qi::Signature CallableType::returnSignature() const
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
        qiLogError() << "convert: unknown type " << (*it).toString();
        compatible = src[idx].type;
      }
      dst.push_back(src[idx].convertCopy(compatible));
    }
    return dst;
  }

  Signature GenericFunctionParameters::signature(bool dyn) const
  {
    const std::vector<GenericValuePtr>& params = *this;
    return qi::makeTupleSignature(params, dyn);
  }

  class DynamicFunctionType: public FunctionType
  {
  public:
    virtual void* call(void* func, void** args, unsigned int argc)
    {
      qiLogError() << "Dynamic function called without type information";
      return 0;
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
    FunctionType* d = dynamicFunctionType();
    GenericFunction result(d, d->clone(d->initializeStorage(&f)));
    return result;
  }

}
#ifdef QITYPE_TRACK_FUNCTIONTYPE_INSTANCES
#include <qi/application.hpp>
namespace qi
{
  namespace detail
  {
    static std::map<std::string, int> functionTrackMap;
    void functionTypeTrack(const std::string& f)
    {
      functionTrackMap[std::string(f)]++;
    }
    void functionTypeDump()
    {
      for (std::map<std::string, int>::iterator it = functionTrackMap.begin();
        it != functionTrackMap.end(); ++it)
        std::cerr << it->second << '\t' << it->first << std::endl;
    }
  }
}
#endif
