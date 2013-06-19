/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include <qi/future.hpp>
#include <qitype/signature.hpp>
#include <qitype/functiontype.hpp>
#include <qitype/genericobject.hpp>

qiLogCategory("qitype.functiontype");

namespace qi
{
  AnyReference AnyFunction::call(
    AnyReference arg1, const std::vector<AnyReference>& remaining)
  {
    std::vector<AnyReference> args;
    args.reserve(remaining.size()+1);
    args.push_back(arg1);
    args.insert(args.end(), remaining.begin(), remaining.end());
    return call(args);
  }

  AnyReference AnyFunction::call(
    const std::vector<AnyReference>& vargs)
  {
    if (type == dynamicFunctionTypeInterface())
    {
      DynamicFunction* f = (DynamicFunction*)value;
      if (!transform.dropFirst && !transform.prependValue)
        return (*f)(vargs);
      std::vector<AnyReference> args;
      if (transform.dropFirst && !transform.prependValue)
      {
        // VCXX2008 does not accept insert here because GV(GVP) ctor is explicit
        args.resize(vargs.size()-1);
        for (unsigned i=0; i<vargs.size()-1; ++i)
          args[i] = vargs[i+1];
      }
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
    const std::vector<TypeInterface*>& target = type->argumentsType();
    unsigned sz = vargs.size();
    const AnyReference* args = sz > 0 ? &vargs[0] : 0;

    if (target.size() != sz + deltaCount)
    {
      throw std::runtime_error(_QI_LOG_FORMAT("Argument count mismatch, expected %s, got %s (transform %s)",
        target.size(), sz, deltaCount));
      return AnyReference();
    }
    if (transform.dropFirst)
    {
      ++args;
      --sz;
    }
    unsigned offset = transform.prependValue? 1:0;
#if QI_HAS_VARIABLE_LENGTH_ARRAY
    AnyReference toDestroy[sz+offset];
    void* convertedArgs[sz+offset];
#else
    AnyReference* toDestroy = new AnyReference[sz+offset];
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
        std::pair<AnyReference,bool> v = args[i].convert(target[i+offset]);
        if (!v.first.type)
        {
          // Try pointer dereference
          if (args[i].kind() == TypeInterface::Pointer)
          {
            AnyReference deref = *const_cast<AnyReference&>(args[i]);
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
            return AnyReference();
          }
        }
        if (v.second)
          toDestroy[toDestroyPos++] = v.first;
        convertedArgs[i+offset] = v.first.value;
      }
    }
    void* res;
    res = type->call(value, convertedArgs, sz+offset);
    AnyReference result;
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

  const AnyFunction& AnyFunction::dropFirstArgument() const
  {
    transform.dropFirst = true;
    return *this;
  }

  const AnyFunction& AnyFunction::prependArgument(void* arg) const
  {
    transform.prependValue = true;
    transform.boundValue = arg;
    return *this;
  }

  const AnyFunction& AnyFunction::replaceFirstArgument(void* arg) const
  {
    transform.dropFirst = true;
    return prependArgument(arg);
  }

  TypeInterface* AnyFunction::resultType() const
  {
    return type->resultType();
  }

  std::vector<TypeInterface*> AnyFunction::argumentsType() const
  {
    std::vector<TypeInterface*> res = type->argumentsType();
    if (transform.dropFirst && transform.prependValue) // optimize that case
      res[0] = typeOf<AnyValue>();
    else if (transform.dropFirst)
    {
      // First argument passed to us will be ignored, so apparent signature
      // has one extra arg of any type.

      // do not access res[1], it might not exist and invalid access might be
      // detected by debug-mode stl
      res.push_back(0);
      memmove(&res[0]+1, &res[0], (res.size()-1)*sizeof(TypeInterface*));
      res[0] = typeOf<AnyValue>();
    }
    else if (transform.prependValue)
    {
      // We bind one argument, so it is not present in apparent signature, remove it
      memmove(&res[0], &res[0]+1, (res.size()-1)*sizeof(TypeInterface*));
      res.pop_back();
    }
    return res;
  }

  qi::Signature AnyFunction::parametersSignature(bool dropFirst) const
  {
    if (type == dynamicFunctionTypeInterface())
      return "m";
    if (!dropFirst)
      return qi::makeTupleSignature(argumentsType());
    std::vector<TypeInterface*> vtype = argumentsType();
    if (vtype.empty())
      throw std::runtime_error("Can't drop the first argument, the argument list is empty");
    //ninja! : drop the first TypeInterface*
    memmove(&vtype[0], &vtype[0]+1, (vtype.size()-1)*sizeof(TypeInterface*));
    vtype.pop_back();
    return qi::makeTupleSignature(vtype);
  }

  qi::Signature AnyFunction::returnSignature() const
  {
    if (type == dynamicFunctionTypeInterface())
      return "m";
    // Detect if returned type is a qi::Future and return underlying value type.
    TemplateTypeInterface* ft1 = QI_TEMPLATE_TYPE_GET(resultType(), Future);
    TemplateTypeInterface* ft2 = QI_TEMPLATE_TYPE_GET(resultType(), FutureSync);
    TemplateTypeInterface* futureType = ft1 ? ft1 : ft2;
    if (futureType)
      return futureType->templateArgument()->signature();
    else
      return resultType()->signature();
  }

  qi::Signature CallableTypeInterface::parametersSignature() const
  {
    if (this == dynamicFunctionTypeInterface())
      return "m";
    else
      return qi::makeTupleSignature(_argumentsType);
  }

  qi::Signature CallableTypeInterface::returnSignature() const
  {
    if (this == dynamicFunctionTypeInterface())
      return "m";
    return _resultType->signature();
  }

  GenericFunctionParameters::GenericFunctionParameters()
  {
  }

  GenericFunctionParameters::GenericFunctionParameters(const std::vector<AnyReference>& args)
  :std::vector<AnyReference>(args)
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
    const std::vector<AnyReference>& src = *this;
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
      TypeInterface* compatible = qi::TypeInterface::fromSignature(*it);
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
    const std::vector<AnyReference>& params = *this;
    return qi::makeTupleSignature(params, dyn);
  }

  class DynamicFunctionTypeInterfaceInterface: public FunctionTypeInterface
  {
  public:
    DynamicFunctionTypeInterfaceInterface()
    {
      _resultType = typeOf<AnyValue>();
    }
    virtual void* call(void* func, void** args, unsigned int argc)
    {
      qiLogError() << "Dynamic function called without type information";
      return 0;
    }
    _QI_BOUNCE_TYPE_METHODS(DefaultTypeImplMethods<DynamicFunction>);
  };

  FunctionTypeInterface* dynamicFunctionTypeInterface()
  {
    static FunctionTypeInterface* type = 0;
    if (!type)
      type = new DynamicFunctionTypeInterfaceInterface();
    return type;
  }

  AnyFunction AnyFunction::fromDynamicFunction(DynamicFunction f)
  {
    FunctionTypeInterface* d = dynamicFunctionTypeInterface();
    AnyFunction result(d, d->clone(d->initializeStorage(&f)));
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
