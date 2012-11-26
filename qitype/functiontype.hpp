#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_FUNCTIONTYPE_HPP_
#define _QITYPE_FUNCTIONTYPE_HPP_

#include <boost/function.hpp>

#include <qitype/type.hpp>

#ifdef _MSC_VER
#  pragma warning( push )
#  pragma warning( disable: 4251 )
#endif

namespace qi {

  /// Signature information for both callable types FunctionType and MethodType
  class QITYPE_API CallableType
  {
  public:
    CallableType();
    Type* resultType();
    const std::vector<Type*>& argumentsType();
    std::string signature() const;
    std::string sigreturn() const;
  protected:
    Type*              _resultType;
    // C4251
    std::vector<Type*> _argumentsType;
  };

  class QITYPE_API FunctionType: public Type, public CallableType
  {
  public:
    /** Call the function func with argument args that must be of the correct type.
    * @return the return value of type resultType(). This value is allocated and must be destroyed.
    */
    virtual void* call(void* storage, void** args, unsigned int argc) = 0;
    /// Default implementation convert arguments to argumentsType()
    /// and bounce to the other call()
    virtual GenericValuePtr call(void* func, const std::vector<GenericValuePtr>& args);
  };

  template<typename T> FunctionType* makeFunctionType();

  /** Represents a generic callable function.
   * This class has value semantic.
  */
  class QITYPE_API GenericFunction
  {
  public:
    GenericFunction();
    ~GenericFunction();
    GenericFunction(const GenericFunction& b);
    GenericFunction& operator = (const GenericFunction& b);
    GenericValuePtr call(const std::vector<GenericValuePtr>& args);
    GenericValuePtr operator()(const std::vector<GenericValuePtr>& args);
    FunctionType* type;
    void* value;
  };


  /** Store function parameters as a list of GenericValuePtr.
  * Storage can be on the stack or allocated
  * Memory management is the responsibility of the user.
  * If GenericFunctionParameters is obtained throug copy(), convert() or
  * fromBuffer(), it must be cleared by destroy()
  */
  class QITYPE_API GenericFunctionParameters: public std::vector<GenericValuePtr>
  {
  public:
    GenericFunctionParameters();
    GenericFunctionParameters(const std::vector<GenericValuePtr>&);
    /// Copy arguments. destroy() must be called on the result
    GenericFunctionParameters copy(bool notFirst=false) const;
    /// Convert the arguments to given signature. destroy() must be called on the result.
    GenericFunctionParameters convert(const Signature& sig) const;
    void destroy(bool notFirst = false);
  };

  typedef boost::function<GenericValuePtr(const std::vector<GenericValuePtr>&)> DynamicFunction;
  /// @return a GenericFunction that takes arguments as a list of unconverted GenericValuePtr.
  QITYPE_API GenericFunction makeDynamicGenericFunction(DynamicFunction f);
  /// @return the type used by dynamic functions
  QITYPE_API FunctionType* dynamicFunctionType();
}

#include <qitype/details/functiontype.hxx>

#ifdef _MSC_VER
#  pragma warning( pop )
#endif

#endif  // _QITYPE_FUNCTIONTYPE_HPP_
