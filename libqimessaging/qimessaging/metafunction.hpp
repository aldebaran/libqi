/*
** Copyright (C) 2012 Aldebaran Robotics
*/


#pragma once

#ifndef _QI_MESSAGING_METAFUNCTION_HH_
#define _QI_MESSAGING_METAFUNCTION_HH_
#include <qimessaging/buffer.hpp>
#include <qimessaging/metatype.hpp>
#include <qimessaging/metavalue.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>

#include <boost/type_traits/add_reference.hpp>
#include <boost/type_traits/remove_pointer.hpp>
#include <boost/function_types/function_type.hpp>
#include <boost/function_types/function_arity.hpp>
#include <boost/function_types/result_type.hpp>
#include <boost/function_types/parameter_types.hpp>
#include <vector>

namespace qi {

/// Internal class that stores MetaValues in various forms.
class QIMESSAGING_API MetaStorage
{
public:
  /* Lifetime of MetaValue is a bit tricky, as a MetaValue can be on the stack,
   * or allocated.
   */
  ~MetaStorage();
  std::vector<MetaValue> parameterValues;
  Buffer                 parameterBuffer;
  std::string            signature;
  bool                   valid;
  bool                   deleteOnDestruction; // delete values if destroyed
  bool                   hasInvalidator;  // True if can be invalidated.
};

/** Manage the various formats of Function parameters, and ensure no
 * lifetime error is made.
 */
class QIMESSAGING_API MetaFunctionParameters
{
public:
  MetaFunctionParameters();
  ~MetaFunctionParameters();
  MetaFunctionParameters(const MetaFunctionParameters& b);
  MetaFunctionParameters& operator = (const MetaFunctionParameters& b);
    /** Set from 'value'.
   * @param invalidateOnDestruction if true, 'value' will be considered invalid
   * when this instance of MetaFunctionParameters is destroyed. Use this
   * when values are on a stack.
   */
  explicit MetaFunctionParameters(const std::vector<MetaValue>& value, bool invalidateOnDestruction=false);
  /// Set from a buffer
  explicit MetaFunctionParameters(Buffer);

  /** Set signature associated with the value.
   *
   * The signature is required to convert buffer to values.
   */
  void setSignature(const std::string& sig);
  const std::vector<MetaValue>& getValues() const;
  const Buffer& getBuffer() const;

  enum Mode
  {
    MODE_BUFFER,
    MODE_METAVALUE
  };
  /// Return the mode available without conversion
  Mode getMode() const;
  /// Make a copy of storage if needed to garantee validity after async call.
  MetaFunctionParameters copy() const;

  /// Convert storage from serialization to value. Sub-optimal.
  void convertToValues() const;
  /// Convert storage to buffer by serializing value.
  void convertToBuffer() const;
protected:
  void  _initStorage();
  mutable boost::shared_ptr<MetaStorage> storage;
  /// True if storage is inalidated when this object is destroyed
  mutable bool                       invalidateOnDestruction;
};

class QIMESSAGING_API MetaFunctionResult: public MetaFunctionParameters
{
public:
  MetaFunctionResult();
  /// Takes ownership of value.
  MetaFunctionResult(const MetaValue& value);
  MetaFunctionResult(Buffer);
  /// Return the value without copying it: Valid until storage goes.
  MetaValue getValue() const;
};

typedef boost::function<MetaFunctionResult(const MetaFunctionParameters&)> MetaFunction;

namespace detail {
template<typename T> MetaFunctionResult metacallBounce(boost::function<T> f,
  const MetaFunctionParameters& params);
}

template<typename T> MetaFunction makeMetaFunction(boost::function<T> f)
{
  return boost::bind(&detail::metacallBounce<T>, f, _1);
}

/*
template<typename F> MetaFunction makeFunctor(F func)
{
  return makeMetaFunction(func);
}

template<typename C, typename F> MetaFunction makeFunctor(C* inst, F func)
{
  typedef typename boost::function_types::result_type<F>::type RetType;
  typedef typename boost::function_types::parameter_types<F>::type MemArgsType;
  typedef typename boost::mpl::pop_front< MemArgsType >::type ArgsType;
  typedef typename boost::mpl::push_front<ArgsType, RetType>::type FullType;
  boost::function<typename boost::function_types::function_type<FullType>::type>
  f; //\ = boost::bind(func, inst);
  return makeMetaFunction(f);
}*/

}

#include <qimessaging/details/makefunctor.hpp>

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


namespace qi {
  namespace detail {
template<typename T> static void deletor(const T* ptr)
{
  delete const_cast<T*>(ptr);
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
    case MetaFunctionParameters::MODE_METAVALUE:
      in = 0;
      break;
    case MetaFunctionParameters::MODE_BUFFER:
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
    if (params.getMode() == MetaFunctionParameters::MODE_METAVALUE)
    {
      std::pair<const T*, bool> res = params.getValues()[idx++].template as<T>();
      if(res.second)
        deletors.push_back(boost::bind(&deletor<T>, const_cast<T*>(res.first)));
      v = const_cast<T*>(res.first);
    }
    else
    {
      T* ptr = new T();
      qiLogDebug("qi.bind") <<" deserializing a " << typeid(*ptr).name() <<" at " << in->getBufferReader().position();
      (*in) >> (*ptr);
      deletors.push_back(boost::bind(&deletor<T>, ptr));
      v = ptr;
    }
  }

  mutable MetaFunctionParameters params;
  mutable int idx;
  mutable IDataStream* in;
  bool alocated;
  std::vector<boost::function<void()> >& deletors;
};

struct PtrToConstRef
{
  template <typename Sig>
  struct result;

  template <class Self, typename T>
  struct result< Self(T) >
  {
    typedef typename boost::add_reference<
    typename boost::add_const<
    typename boost::remove_pointer<
    typename boost::remove_reference<T>::type>::type>::type>::type type;
  };
  template<typename T>
  const T& operator() (T* ptr) const
  {
    return *ptr;
  }
};


/* Use a helper function to avoid having to express the type SEQ,
 */
template<typename SEQ, typename F> static MetaValue apply(SEQ sequence,
  ArgTransformer& argumentTransformer,
  F& function)
{
  // Deserialize arguments
   boost::fusion::for_each(sequence, argumentTransformer);
   MetaValueCopy res;
  // Invoke our function pointer.
  res, boost::fusion::invoke_function_object(function,
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

  MetaValueCopy res;
  ArgTransformer argumentTransformer(&deletors, params);
  qiLogDebug("qi.bind") << "ArgTransformer(" << &argumentTransformer <<") is go";

  // Create a vector of T* with whatever value. Since we cannot express its
  // type, do the rest in a helper function.
  MetaValue result = apply(
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
