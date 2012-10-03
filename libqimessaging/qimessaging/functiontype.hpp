#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QIMESSAGING_FUNCTIONTYPE_HPP_
#define _QIMESSAGING_FUNCTIONTYPE_HPP_

#include <boost/function.hpp>

#include <qimessaging/type.hpp>

namespace qi {

  class QIMESSAGING_API CallableType
  {
  public:
    Type* resultType();
    const std::vector<Type*>& argumentsType();
    std::string signature() const;
    std::string sigreturn() const;
  protected:
    Type*              _resultType;
    std::vector<Type*> _argumentsType;
  };

  class QIMESSAGING_API FunctionType: public Type, public CallableType
  {
  public:
    virtual void* call(void* func, const std::vector<void*>& args) = 0;
    /// Default implementation convert arguments to argumentsType()
    /// and bounce to the other call()
    virtual GenericValue call(void* func, const std::vector<GenericValue>& args);
  };

  template<typename T> FunctionType* makeFunctionType();

  /** Represents a generic callable function.
   * This class has value semantic.
  */
  class QIMESSAGING_API GenericFunction
  {
  public:
    GenericFunction();
    GenericValue call(const std::vector<GenericValue>& args);
    GenericValue operator()(const std::vector<GenericValue>& args);
    FunctionType* type;
    boost::function<void ()> value;
  };

  template<typename T> GenericFunction makeGenericFunction(boost::function<T> f);
  template<typename F> GenericFunction makeGenericFunction(F func);

  typedef boost::function<GenericValue(const std::vector<GenericValue>&)> DynamicFunction;
  /// @return a GenericFunction that takes arguments as a list of unconverted GenericValue.
  QIMESSAGING_API GenericFunction makeDynamicGenericFunction(DynamicFunction f);

  template<typename O, typename F> GenericFunction makeGenericFunction(O o, F f);

  /** Store function parameters as a list of GenericValue.
  * Storage can be on the stack or allocated
  *
  */
  class QIMESSAGING_API GenericFunctionParameters: public std::vector<GenericValue>
  {
  public:
    GenericFunctionParameters();
    GenericFunctionParameters(const std::vector<GenericValue>&);
    /// Copy arguments. destroy() must be called on the result
    GenericFunctionParameters copy(bool notFirst=false) const;
    /// Convert the arguments to given signature. destroy() must be called on the result.
    GenericFunctionParameters convert(const Signature& sig) const;
    void destroy(bool notFirst = false);
    /// Extract from buffer and signature. Throws on error
    static GenericFunctionParameters fromBuffer(const Signature& sig, const qi::Buffer& buf);
    /// Serialize to buffer
    qi::Buffer toBuffer() const;
  };
}

#include <qimessaging/details/functiontype.hxx>

#endif  // _QIMESSAGING_FUNCTIONTYPE_HPP_
