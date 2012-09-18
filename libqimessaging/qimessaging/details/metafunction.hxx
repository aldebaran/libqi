/*
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef _QI_MESSAGING_METAFUNCTION_HXX_
#define _QI_MESSAGING_METAFUNCTION_HXX_

#ifdef BOOST_FUSION_INVOKE_FUNCTION_OBJECT_MAX_ARITY
# undef BOOST_FUSION_INVOKE_FUNCTION_OBJECT_MAX_ARITY
#endif
#define BOOST_FUSION_INVOKE_FUNCTION_OBJECT_MAX_ARITY 10

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


template<typename T, bool b> struct MakeCallableSwitch
{
};


template<typename T> struct MakeCallableSwitch<T, true>
{
  MetaCallable operator()(T fun)
  {
    return makeCallable(makeGenericMethod(fun));
  }
};

template<typename T> struct MakeCallableSwitch<T, false>
{
  MetaCallable operator()(T fun)
  {
  return makeCallable(makeGenericFunction(fun));
  }
};

template<typename T> MetaCallable makeCallable(T fun)
{
  return MakeCallableSwitch<T,
    boost::function_types::is_member_function_pointer<T>::value>()(fun);
}

template<> inline MetaCallable makeCallable(MetaCallable fun)
{
  return fun;
}


}


#endif
