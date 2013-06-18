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

  /// Signature information for both callable types FunctionTypeInterface and MethodType
  class QITYPE_API CallableType
  {
  public:
    CallableType();
    Type* resultType();
    const std::vector<Type*>& argumentsType();
    qi::Signature parametersSignature() const;
    qi::Signature returnSignature() const;
  protected:
    Type*              _resultType;
    // C4251
    std::vector<Type*> _argumentsType;
  };

  class QITYPE_API FunctionTypeInterface: public Type, public CallableType
  {
  public:
    /** Call the function func with argument args that must be of the correct type.
    * @return the return value of type resultType(). This value is allocated and must be destroyed.
    */
    virtual void* call(void* storage, void** args, unsigned int argc) = 0;
  };

  template<typename T> FunctionTypeInterface* makeFunctionTypeInterface();

  struct ArgumentTransformation
  {
  public:
    // Drop first argument
    bool dropFirst;
    // Prepend boundValue to argument list
    bool prependValue;

    // So if both dropFirst and prependValue are set, first argument is
    // replaced with boundValue.
    ArgumentTransformation(bool dropFirst = false, bool prependValue=false, void* value = 0)
    : dropFirst(dropFirst)
    , prependValue(prependValue)
    , boundValue(value)
    {}

    void* boundValue;
  };

  /// A function with AnyArguments as its sole argument will behave as if AnyFunction::fromDynamicFunction was called.
  class QITYPE_API AnyArguments
  {
  public:
    AnyArguments() {};
    AnyArguments(const std::vector<GenericValue>& args)
    : _args(args) {}
    operator const std::vector<GenericValue>&() const { return _args;}
    std::vector<GenericValue> &args()             { return _args; }
    const std::vector<GenericValue> &args() const { return _args; }

  private:
    std::vector<GenericValue> _args;
  };

  typedef boost::function<GenericValuePtr(const std::vector<GenericValuePtr>&)> DynamicFunction;

  /** Represents a generic callable function.
   * This class has value semantic.
   *
   */
  class QITYPE_API AnyFunction
  {
  public:
    AnyFunction();
    ~AnyFunction();
    AnyFunction(const AnyFunction& b);
    AnyFunction(FunctionTypeInterface* type, void* value);
    AnyFunction& operator = (const AnyFunction& b);
    GenericValuePtr call(const std::vector<GenericValuePtr>& args);
    GenericValuePtr call(GenericValuePtr arg1, const std::vector<GenericValuePtr>& args);
    GenericValuePtr operator()(const std::vector<GenericValuePtr>& args);

    /// Change signature, drop the first argument passed to call.
    const AnyFunction& dropFirstArgument() const;
    /// Replace first argument by \p value which must be storage for correct type.
    const AnyFunction& replaceFirstArgument(void* value) const;
    /// Prepend extra argument \p value to argument list
    const AnyFunction& prependArgument(void* value) const;

    /// Return expected argument types, taking transform into account
    std::vector<Type*> argumentsType() const;
    Type*              resultType() const;
    //dropfirst is useful when you want the parameters signature of a method.
    Signature          parametersSignature(bool dropFirst=false) const;
    Signature          returnSignature() const;

    void swap(AnyFunction& b);

    operator bool() const;
    FunctionTypeInterface* functionType() const;

    /*** @return an AnyFunction wrapping func.
    * func can be:
    * - a boost::bind object
    * - a boost::function
    * - a function pointer
    * - a member function pointer
    *
    */
    template<typename F>
    static AnyFunction from(F func);
    /// @return a AnyFunction binding \p instance to member function \p func
    template<typename F, typename C>
    static AnyFunction from(F func, C instance);


    /// @return a AnyFunction that takes arguments as a list of unconverted GenericValuePtr.
    static AnyFunction fromDynamicFunction(DynamicFunction f);

  private:
    FunctionTypeInterface*  type;
    void* value; //type-dependant storage
    mutable ArgumentTransformation transform;
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
    qi::Signature signature(bool dyn) const;
    void destroy(bool notFirst = false);
  };

  /// @return the type used by dynamic functions
  QITYPE_API FunctionTypeInterface* dynamicFunctionTypeInterface();
}

#include <qitype/details/functiontype.hxx>
#include <qitype/details/functiontypefactory.hxx>
#ifdef _MSC_VER
#  pragma warning( pop )
#endif

#endif  // _QITYPE_FUNCTIONTYPE_HPP_
