#ifndef _QIMESSAGING_METHODTYPE_HXX_
#define _QIMESSAGING_METHODTYPE_HXX_

namespace qi
{
  namespace detail
  {
    // Convert method signature to function signature by putting
    // "class instance pointer" type as first argument
    // ex: int (Foo::*)(int) => int (*)(Foo*, int)
    template<typename F>
    struct MethodToFunctionTrait
    {
      // Result type
      typedef typename ::boost::function_types::result_type<F>::type RetType;
      // All arguments including class pointer
      typedef typename ::boost::function_types::parameter_types<F>::type ArgsType;
      // Class ref type
      typedef typename ::boost::mpl::front<ArgsType>::type ClassRefType;
      // Convert it to ptr type
      typedef typename boost::add_pointer<typename boost::remove_reference<ClassRefType>::type>::type ClassPtrType;
      // Argument list, changing ClassRef to ClassPtr
      typedef typename boost::mpl::push_front<
      typename ::boost::mpl::pop_front<ArgsType>::type,
      ClassPtrType>::type ArgsTypeFixed;
      // Push result type in front
      typedef typename ::boost::mpl::push_front<ArgsTypeFixed, RetType>::type FullType;
      // Synthetise result function type
      typedef typename ::boost::function_types::function_type<FullType>::type type;
    };
  } // namespace detail

  template<typename T>
  class MethodTypeImpl:
    public virtual MethodType,
    public virtual FunctionTypeImpl<T>
  {
    void* call(void* method, void* object,
      const std::vector<void*>& args)
    {
      std::vector<void*> nargs;
      nargs.reserve(args.size()+1);
      nargs.push_back(object);
      nargs.insert(nargs.end(), args.begin(), args.end());
      return FunctionTypeImpl<T>::call(method, nargs);
    }
    Value call(void* method, Value object,
      const std::vector<Value>& args)
    {
      std::vector<Value> nargs;
      nargs.reserve(args.size()+1);
      nargs.push_back(object);
      nargs.insert(nargs.end(), args.begin(), args.end());
      return FunctionType::call(method, nargs);
    }
  };

  template<typename T>
  MethodType* methodTypeOf()
  {
    static MethodTypeImpl<T> result;
    return &result;
  }
  template<typename M>
  MethodValue makeMethodValue(const M& method)
  {
    // convert M to a boost::function with an extra arg object_type
    typedef typename detail::MethodToFunctionTrait<M>::type Linearized;
    boost::function<Linearized> f = method;

    FunctionValue fv = makeFunctionValue(f);
    MethodValue result;
    result.value = fv.value;
    result.type = methodTypeOf<Linearized>();
    return result;
  }
} // namespace qi

#endif
