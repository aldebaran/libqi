#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_FUNCTIONTYPE_HPP_
#define _QITYPE_FUNCTIONTYPE_HPP_

#include <boost/function.hpp>

#include <qitype/type.hpp>

namespace qi {

  /// Signature information for both callable types FunctionType and MethodType
  class QITYPE_API CallableType
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

  class QITYPE_API FunctionType: public Type, public CallableType
  {
  public:
    /** Call the function func with argument args that must be of the correct type.
    * @return the return value of type resultType(). This value is allocated and must be destroyed.
    */
    virtual void* call(void* func, void** args, unsigned int argc) = 0;
    /// Default implementation convert arguments to argumentsType()
    /// and bounce to the other call()
    virtual GenericValue call(void* func, const std::vector<GenericValue>& args);
  };

  template<typename T> FunctionType* makeFunctionType();

  /** Represents a generic callable function.
   * This class has value semantic.
  */
  class QITYPE_API GenericFunction
  {
  public:
    GenericFunction();
    GenericValue call(const std::vector<GenericValue>& args);
    GenericValue operator()(const std::vector<GenericValue>& args);
    FunctionType* type;
    boost::function<void ()> value;
  };

 template<typename F> GenericFunction makeGenericFunction(F func);

  typedef boost::function<GenericValue(const std::vector<GenericValue>&)> DynamicFunction;
  /// @return a GenericFunction that takes arguments as a list of unconverted GenericValue.
  QITYPE_API GenericFunction makeDynamicGenericFunction(DynamicFunction f);

  /// @return a GenericFunction obtained by binding a class instance to a member function
  template<typename O, typename F> GenericFunction makeGenericFunction(O o, F f);

  /** Store function parameters as a list of GenericValue.
  * Storage can be on the stack or allocated
  * Memory management is the responsibility of the user.
  * If GenericFunctionParameters is obtained throug copy(), convert() or
  * fromBuffer(), it must be cleared by destroy()
  */
  class QITYPE_API GenericFunctionParameters: public std::vector<GenericValue>
  {
  public:
    GenericFunctionParameters();
    GenericFunctionParameters(const std::vector<GenericValue>&);
    /// Copy arguments. destroy() must be called on the result
    GenericFunctionParameters copy(bool notFirst=false) const;
    /// Convert the arguments to given signature. destroy() must be called on the result.
    GenericFunctionParameters convert(const Signature& sig) const;
    void destroy(bool notFirst = false);
  };
}

#include <qitype/details/functiontype.hxx>

#endif  // _QITYPE_FUNCTIONTYPE_HPP_
