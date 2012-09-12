/*
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef _QI_MESSAGING_METAFUNCTION_HXX_
#define _QI_MESSAGING_METAFUNCTION_HXX_

#ifdef BOOST_FUSION_INVOKE_FUNCTION_OBJECT_MAX_ARITY
# undef BOOST_FUSION_INVOKE_FUNCTION_OBJECT_MAX_ARITY
#endif
#define BOOST_FUSION_INVOKE_FUNCTION_OBJECT_MAX_ARITY 10

#include <qimessaging/method_type.hpp>

#include <boost/fusion/include/mpl.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/transform_view.hpp>
#include <boost/type_traits/remove_reference.hpp>
#include <boost/type_traits/add_pointer.hpp>
#include <boost/type_traits/remove_const.hpp>
#include <boost/function_types/function_type.hpp>
#include <boost/function_types/result_type.hpp>
#include <boost/function_types/parameter_types.hpp>
#include <boost/fusion/container/vector/convert.hpp>
#include <boost/fusion/include/as_vector.hpp>
#include <boost/fusion/include/as_list.hpp>
#include <boost/fusion/algorithm/transformation/transform.hpp>
#include <boost/fusion/include/transform.hpp>
#include <boost/fusion/functional/invocation/invoke_function_object.hpp>
#include <boost/fusion/container/generation/make_vector.hpp>
#include <boost/fusion/include/make_vector.hpp>
#include <boost/fusion/algorithm/iteration/for_each.hpp>
#include <boost/fusion/functional/adapter/unfused.hpp>
#include <boost/fusion/functional/generation/make_unfused.hpp>
#include <boost/fusion/functional/generation/make_fused.hpp>

namespace qi {
  namespace detail {
    template<typename T> MetaFunctionResult metacallBounce(boost::function<T> f,
      const MetaFunctionParameters& params);

  template<typename T> MetaFunction makeMetaFunction(boost::function<T> f)
  {
    return boost::bind(&detail::metacallBounce<T>, f, _1);
  }
}


template<typename F> MetaFunction makeFunctor(boost::function<F> func)
{
  return detail::makeMetaFunction(func);
}

template<typename F> MetaFunction makeFunctor(F func)
{
  return detail::makeMetaFunction(boost::function<
    typename boost::remove_pointer<F>::type>(func));
}


namespace detail
{
  /* Call a boost::function<F> binding the first argument.
  * Can't be done just with boost::bind without code generation.
  */
  template<typename F>
  struct FusedBindOne
  {
    template <class Seq>
    struct result
    {
      typedef typename boost::function_types::result_type<F>::type type;
    };

    template <class Seq>
    typename result<Seq>::type
    operator()(Seq const & s) const
    {
      return ::boost::fusion::invoke_function_object(func,
        ::boost::fusion::push_front(s, boost::ref(const_cast<ArgType&>(*arg1))));
    }
    ::boost::function<F> func;
    typedef typename boost::remove_reference<
      typename ::boost::mpl::front<
        typename ::boost::function_types::parameter_types<F>::type
        >::type>::type ArgType;
    void setArg(ArgType* val) { arg1 = val;}
    ArgType* arg1;

  };

}
template<typename C, typename F> MetaFunction makeFunctor(C* inst, F func)
{
  // Return type
  typedef typename ::boost::function_types::result_type<F>::type RetType;
  // All arguments including class pointer
  typedef typename ::boost::function_types::parameter_types<F>::type MemArgsType;
  // Pop class pointer
  typedef typename ::boost::mpl::pop_front< MemArgsType >::type ArgsType;
  // Synthethise exposed function type
  typedef typename ::boost::mpl::push_front<ArgsType, RetType>::type ResultMPLType;
  typedef typename ::boost::function_types::function_type<ResultMPLType>::type ResultType;
  // Synthethise non-member function equivalent type of F
  typedef typename ::boost::mpl::push_front<MemArgsType, RetType>::type MemMPLType;
  typedef typename ::boost::function_types::function_type<MemMPLType>::type LinearizedType;
  // See func as R (C*, OTHER_ARGS)
  boost::function<LinearizedType> memberFunction = func;
  boost::function<ResultType> res;
  // Create the fusor
  detail::FusedBindOne<LinearizedType> fusor;
  // Bind member function and instance
  fusor.setArg(inst);
  fusor.func = memberFunction;
  // Convert it to a boost::function
  res = boost::fusion::make_unfused(fusor);

  return detail::makeMetaFunction(res);
}

namespace detail {

// We need a metatype deletor and a type deletor


template<typename T> inline void deletor(Type* type, void* ptr)
{
  type->destroy(ptr);
}

template<> inline void deletor<std::vector<Value> >(Type* type, void* ptr)
{
  std::vector<Value>* val = (std::vector<Value>*)ptr;
  for (unsigned i=0; i<val->size(); ++i)
    (*val)[i].destroy();
  type->destroy(ptr);
}




template<typename T> inline void deletorT(T* ptr)
{
  delete ptr;
}

template<> inline void deletorT<Value>(Value* ptr)
{ // Bounce to the other one
  deletor<Value>(typeOf<Value>(), ptr);
}

template<> inline void deletorT<std::vector<Value> >(std::vector<Value>* ptr)
{
  deletor<std::vector<Value> >(typeOf<std::vector<Value> >(), (void*)ptr);
}


struct ArgTransformer
{
  ArgTransformer(std::vector<boost::function<void()> >* d,
    const MetaFunctionParameters& p)
  : alocated(true)
  , deletors(*d)
  {
    params = const_cast<MetaFunctionParameters&>(p);
    idx = 0;
    switch(params.getMode())
    {
    case MetaFunctionParameters::Mode_Value:
      in = 0;
      break;
    case MetaFunctionParameters::Mode_Buffer:
      idx = -1;
      in = new IDataStream(params.getBuffer());
    }
  }
  ArgTransformer(const ArgTransformer& b)
  : params(b.params)
  , idx(b.idx)
  , in(b.in)
  , alocated(false)
  , deletors(b.deletors)
  {

  }
  ~ArgTransformer()
  {
    if (alocated)
      delete in;
  }

  template <typename Sig>
  struct result;

  template <class Self, typename T>
  struct result< Self(T) >
  {
    typedef T type;
  };

  template<typename T>
  void
  operator() (T* &v) const
  {
    qiLogDebug("qi.bind") << " ArgTransformer(" << this <<") on " << typeid(&v).name();
    if (params.getMode() == MetaFunctionParameters::Mode_Value)
    {
      std::pair<const T*, bool> res = params.getValues()[idx++].template as<T>();
      if(res.second)
        deletors.push_back(boost::bind(&deletorT<T>, const_cast<T*>(res.first)));
      v = const_cast<T*>(res.first);
    }
    else
    {
      qiLogDebug("qi.bind") <<" deserializing a " << typeid(v).name() <<" at " << in->getBufferReader().position();
      // Serialize operator might not exist, go through MetaType
      Type* type = typeOf<T>();
      void* value = type->deserialize(*in);
      if (!value)
      {
        qiLogError("qi.bin") << "Deserialization failure";
      }
      else
      {
        deletors.push_back(boost::bind(&deletor<T>, type, value));
        v = (T*)value;
      }
    }
    qiLogDebug("qi.bind") << "value is " << v;
  }

  mutable MetaFunctionParameters params;
  mutable int idx;
  mutable IDataStream* in;
  bool alocated;
  std::vector<boost::function<void()> >& deletors;
};

/* Use a helper function to avoid having to express the type SEQ,
 */
template<typename SEQ, typename F> static Value apply(SEQ sequence,
  ArgTransformer& argumentTransformer,
  F& function)
{
  // Deserialize arguments
   boost::fusion::for_each(sequence, argumentTransformer);
   ValueCopy res;
  // Invoke our function pointer.
   res(), boost::fusion::invoke_function_object(function,
    boost::fusion::transform(sequence,
      PtrToConstRef()));
  return res;
}

template<typename T> MetaFunctionResult
metacallBounce(boost::function<T> f, const MetaFunctionParameters& params)
{

  /* Warning: transform returns a lazy view. So if you pass it to
  * invoke_function_object, evaluation will happen right to left.
  * But ArgTransformer expects left to right. So use a not-lazy approach by
  * going through for_each
  */
  typedef typename boost::function_types::parameter_types<T>::type ArgsType;
  typedef typename  boost::mpl::transform_view<ArgsType,
    boost::remove_const<
    boost::remove_reference<boost::mpl::_1> > >::type BareArgsType;
  typedef typename boost::mpl::transform_view<BareArgsType,
    boost::add_pointer<boost::mpl::_1> >::type PtrArgsType;

  // Destruction operatiorns to perform when done
  std::vector<boost::function<void() > > deletors;

  ValueCopy res;
  ArgTransformer argumentTransformer(&deletors, params);
  qiLogDebug("qi.bind") << "ArgTransformer(" << &argumentTransformer <<") is go";

  // Create a vector of T* with whatever value. Since we cannot express its
  // type, do the rest in a helper function.
  Value result = apply(
    boost::fusion::as_vector(PtrArgsType()),
    argumentTransformer, f);
  // Apply registered destruction functions
  for (unsigned i=0; i<deletors.size(); ++i)
    deletors[i]();
  return MetaFunctionResult(result);
}
}
}


#endif
