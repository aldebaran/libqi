#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_ANYFUNCTION_HPP_
#define _QITYPE_ANYFUNCTION_HPP_

#include <boost/function.hpp>

#include <qitype/typeinterface.hpp>

#ifdef _MSC_VER
#  pragma warning( push )
#  pragma warning( disable: 4251 )
#endif

namespace qi {

  /// Signature information for both callable types FunctionTypeInterface and MethodType
  class QITYPE_API CallableTypeInterface
  {
  public:
    CallableTypeInterface();
    TypeInterface* resultType();
    const std::vector<TypeInterface*>& argumentsType();
    qi::Signature parametersSignature() const;
    qi::Signature returnSignature() const;
  protected:
    TypeInterface*              _resultType;
    // C4251
    std::vector<TypeInterface*> _argumentsType;
  };

  class QITYPE_API FunctionTypeInterface: public TypeInterface, public CallableTypeInterface
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
    AnyArguments(const std::vector<AnyValue>& args)
    : _args(args) {}
    operator const std::vector<AnyValue>&() const { return _args;}
    std::vector<AnyValue> &args()             { return _args; }
    const std::vector<AnyValue> &args() const { return _args; }

  private:
    std::vector<AnyValue> _args;
  };

  typedef boost::function<AnyReference(const std::vector<AnyReference>&)> DynamicFunction;

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
    AnyReference call(const std::vector<AnyReference>& args);
    AnyReference call(AnyReference arg1, const std::vector<AnyReference>& args);
    AnyReference operator()(const std::vector<AnyReference>& args);

    /// Change signature, drop the first argument passed to call.
    const AnyFunction& dropFirstArgument() const;
    /// Replace first argument by \p value which must be storage for correct type.
    const AnyFunction& replaceFirstArgument(void* value) const;
    /// Prepend extra argument \p value to argument list
    const AnyFunction& prependArgument(void* value) const;

    /// Return expected argument types, taking transform into account
    std::vector<TypeInterface*> argumentsType() const;
    TypeInterface*              resultType() const;
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


    /// @return a AnyFunction that takes arguments as a list of unconverted AnyReference.
    static AnyFunction fromDynamicFunction(DynamicFunction f);

  private:
    FunctionTypeInterface*  type;
    void* value; //type-dependant storage
    mutable ArgumentTransformation transform;
  };


  /** Store function parameters as a list of AnyReference.
   * Storage can be on the stack or allocated
   * Memory management is the responsibility of the user.
   * If GenericFunctionParameters is obtained throug copy(), convert() or
   * fromBuffer(), it must be cleared by destroy()
   */
  class QITYPE_API GenericFunctionParameters: public std::vector<AnyReference>
  {
  public:
    GenericFunctionParameters();
    GenericFunctionParameters(const std::vector<AnyReference>&);
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

#include <qitype/details/anyfunction.hxx>
#include <qitype/details/anyfunctionfactory.hxx>

#ifdef _MSC_VER
#  pragma warning( pop )
#endif

#endif  // _QITYPE_ANYFUNCTION_HPP_
