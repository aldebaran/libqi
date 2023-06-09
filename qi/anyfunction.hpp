#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QI_ANYFUNCTION_HPP_
#define _QI_ANYFUNCTION_HPP_

#include <qi/api.hpp>
#include <ka/macro.hpp>
#include <boost/function.hpp>
#include <vector>

namespace qi{
  class AnyValue;
  class AutoAnyReference;

  template <typename T = AnyValue>
  class VarArguments {
  public:
    VarArguments() {};
    VarArguments(const T& t) { _args.push_back(t); }
    VarArguments& operator()(const T& t) { _args.push_back(t);  return *this; }

    using VectorType = std::vector<T>;

    VectorType &args()             { return _args; }
    const VectorType &args() const { return _args; }

  private:
    VectorType _args;
  };

  template <>
  class VarArguments<AnyValue> {
  public:
    VarArguments() {};
    VarArguments(const AutoAnyReference& t);
    VarArguments& operator()(const AutoAnyReference& t);

    using VectorType = std::vector<AnyValue>;

    VectorType &args()             { return _args; }
    const VectorType &args() const { return _args; }

  private:
    VectorType _args;
  };

  using AnyVarArguments = VarArguments<>;
}

#include <qi/type/typeinterface.hpp>


KA_WARNING_PUSH()
KA_WARNING_DISABLE(4251, )

namespace qi {

  inline VarArguments<AnyValue>::VarArguments(const AutoAnyReference& t) {
    _args.push_back(qi::AnyValue(t));
  }

  inline VarArguments<AnyValue>& VarArguments<AnyValue>::operator()(const AutoAnyReference& t) {
    _args.push_back(qi::AnyValue(t));
    return *this;
  }


  /// Signature information for both callable types FunctionTypeInterface and MethodType
  class QI_API CallableTypeInterface
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

  class QI_API FunctionTypeInterface: public TypeInterface, public CallableTypeInterface
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

  template <typename T = AnyValue>
  class QI_API KeywordArguments {
  public:
    KeywordArguments& operator()(const std::string& name, const T& t) { values[name] = t; return *this; }

    std::map<std::string, T> values;
  };

  /// A function with AnyArguments as its sole argument will behave as if AnyFunction::fromDynamicFunction was called.
  // This is going to be deprecated in profit of VarArgument and AnyVarArgument
  class QI_API AnyArguments
  {
  public:
    AnyArguments() {};
    AnyArguments(const AnyValueVector& args)
    : _args(args) {}
    operator const AnyValueVector&() const { return _args;}
    AnyValueVector &args()             { return _args; }
    const AnyValueVector &args() const { return _args; }

  private:
    AnyValueVector _args;
  };

  using DynamicFunction = boost::function<AnyReference(const AnyReferenceVector&)>;

  /** Represents a generic callable function.
   * This class has value semantic.
   *
   * \includename{qi/anyfunction.hpp}
   */
  class QI_API AnyFunction
  {
  public:
    AnyFunction();
    ~AnyFunction();
    AnyFunction(const AnyFunction& b);
    AnyFunction(FunctionTypeInterface* type, void* value);
    AnyFunction& operator = (const AnyFunction& b);

    /// Calls the function.
    /// @param args A list of unnamed arguments, each wrapped in an
    /// AnyReference for allowing introspection.
    /// @throw If an argument mismatches the signature, or is invalid.
    AnyReference call(const AnyReferenceVector& args);
    /// Call the function, reference must be destroy()ed
    AnyReference call(AnyReference arg1, const AnyReferenceVector& args);
    /// Call the function, reference must be destroy()ed
    AnyReference operator()(const AnyReferenceVector& args);

#ifdef DOXYGEN
  /// Call the function
  template <typename R>
  R call(
         qi::AutoAnyReference p1 = qi::AutoAnyReference(),
         qi::AutoAnyReference p2 = qi::AutoAnyReference(),
         qi::AutoAnyReference p3 = qi::AutoAnyReference(),
         qi::AutoAnyReference p4 = qi::AutoAnyReference(),
         qi::AutoAnyReference p5 = qi::AutoAnyReference(),
         qi::AutoAnyReference p6 = qi::AutoAnyReference(),
         qi::AutoAnyReference p7 = qi::AutoAnyReference(),
         qi::AutoAnyReference p8 = qi::AutoAnyReference());

  /// Call the function, reference must be destroy()ed
  template<typename R>
  AnyReference operator()(
                          qi::AutoAnyReference p1 = qi::AutoAnyReference(),
                          qi::AutoAnyReference p2 = qi::AutoAnyReference(),
                          qi::AutoAnyReference p3 = qi::AutoAnyReference(),
                          qi::AutoAnyReference p4 = qi::AutoAnyReference(),
                          qi::AutoAnyReference p5 = qi::AutoAnyReference(),
                          qi::AutoAnyReference p6 = qi::AutoAnyReference(),
                          qi::AutoAnyReference p7 = qi::AutoAnyReference(),
                          qi::AutoAnyReference p8 = qi::AutoAnyReference());
#else
#define pushi(z, n, _) params.push_back(p ## n);
#define genCall(n, ATYPEDECL, ATYPES, ADECL, AUSE, comma) \
  template <typename R> R call(                           \
      QI_GEN_ARGSDECLSAMETYPE(n, qi::AutoAnyReference))   \
  {                                                       \
    AnyValue ret(this->operator()(AUSE), false, true);    \
    return ret.to<R>();                                   \
  }                                                       \
  AnyReference operator()(                                \
      QI_GEN_ARGSDECLSAMETYPE(n, qi::AutoAnyReference))   \
  {                                                       \
    std::vector<qi::AnyReference> params;                 \
    params.reserve(n);                                    \
    BOOST_PP_REPEAT(n, pushi, _)                          \
    return call(params);                                  \
  }
QI_GEN(genCall)
#undef genCall
#undef pushi
#endif

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
    static AnyFunction from(F&& func);
    /// @return a AnyFunction binding instance to member function func
    template<typename F, typename C>
    static AnyFunction from(F&& func, C instance);


    /** @return a AnyFunction that takes arguments as a list of unconverted
     * AnyReference.
     */
    static AnyFunction fromDynamicFunction(DynamicFunction f);

  private:
    FunctionTypeInterface* type;
    void* value; //type-dependant storage
    mutable ArgumentTransformation transform;
  };


  /** Store function parameters as a list of AnyReference.
   * Storage can be on the stack or allocated
   * Memory management is the responsibility of the user.
   * If GenericFunctionParameters is obtained throug copy(), convert() or
   * fromBuffer(), it must be cleared by destroy()
   */
  class QI_API GenericFunctionParameters: public AnyReferenceVector
  {
  public:
    GenericFunctionParameters();
    GenericFunctionParameters(const AnyReferenceVector&);
    /// Copy arguments. destroy() must be called on the result
    GenericFunctionParameters copy(bool notFirst=false) const;
    /// Convert the arguments to given signature. destroy() must be called on the result.
    GenericFunctionParameters convert(const Signature& sig) const;
    qi::Signature signature(bool dyn) const;
    void destroy(bool notFirst = false);
  };

  /// @return the type used by dynamic functions
  QI_API FunctionTypeInterface* dynamicFunctionTypeInterface();

namespace detail
{

  // This is just a hint of the maximum of the number of arguments that can be passed to a function,
  // that is used to preallocate on the stack some of the arguments containers. It should be a
  // number that will cover, if not all, most cases of functions without being too big that it would
  // be unused stack space.
  const std::size_t maxAnyFunctionArgsCountHint = 8u;

}
}

#include <qi/type/detail/anyfunction.hxx>
#include <qi/type/detail/anyfunctionfactory.hxx>

KA_WARNING_POP()

#endif  // _QITYPE_ANYFUNCTION_HPP_
