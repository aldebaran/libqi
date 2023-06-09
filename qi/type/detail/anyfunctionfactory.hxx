#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_DETAIL_ANYFUNCTIONFACTORY_HXX_
#define _QITYPE_DETAIL_ANYFUNCTIONFACTORY_HXX_

#include <type_traits>
#include <functional>
#include <tuple>
#include <utility>
#include <boost/callable_traits.hpp>
#define BOOST_BIND_GLOBAL_PLACEHOLDERS
#include <boost/bind.hpp>
#undef BOOST_BIND_GLOBAL_PLACEHOLDERS
#include <boost/any.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/callable_traits.hpp>
#include <qi/atomic.hpp>
#include <qi/anyvalue.hpp>
#include <qi/anyfunction.hpp>
#include <qi/type/detail/bindtype.hxx>
#include <qi/assert.hpp>
#include <qi/macro.hpp>
#include <ka/macro.hpp>
#include <ka/typetraits.hpp>

namespace qi
{

KA_WARNING_PUSH()
KA_WARNING_DISABLE(4068, pragmas)
KA_WARNING_DISABLE(, noexcept-type)

  namespace detail::anyfunction
  {
    template<typename... T>
    std::vector<TypeInterface*> argumentTypes(ka::type_t<T...>)
    {
      return { qi::typeOf<std::remove_const_t<std::remove_reference_t<T>>>()... };
    }

    template<typename F, typename... Args>
    AnyReference invokeIntoAnyReference(F&& f, Args&&... args)
    {
      if constexpr (std::is_void_v<std::invoke_result_t<F&&, Args&&...>>) {
        std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
        return AnyReference(qi::typeOf<void>());
      }
      else {
        return AnyReference::from(
                   std::invoke(std::forward<F>(f), std::forward<Args>(args)...))
            .clone();
      }
    }

    template<typename T>
    T& castMakeCallArg(void*& arg)
    {
      static const auto type = typeOf<T>();
      return *static_cast<std::add_pointer_t<T>>(type->ptrFromStorage(&arg));
    }

    template<typename Fn, typename Args, std::size_t... Index>
    void* makeCall(ka::type_t<Args>,
                   std::index_sequence<Index...>,
                   Fn&& fn,
                   [[maybe_unused]] std::vector<void*>&& args)
    {
      const auto resRef = invokeIntoAnyReference(
          std::forward<Fn>(fn),
          castMakeCallArg<std::tuple_element_t<Index, Args>>(args.at(Index))...);
      return resRef.rawValue();
    }

    template<typename Fn>
    void* makeCall(Fn&& fn, std::vector<void*>&& args)
    {
      using Args = boost::callable_traits::args_t<Fn>;
      return makeCall(ka::type_t<Args>(),
                      std::make_index_sequence<std::tuple_size_v<Args>>(),
                      std::forward<Fn>(fn), std::move(args));
    }

#ifdef QITYPE_TRACK_FUNCTIONTYPE_INSTANCES
    // debug-tool to monitor function type usage
    void QI_API functionTypeTrack(const std::string& functionName);
    void QI_API functionTypeDump();
#endif

    template<typename Fn>
    class FunctionType: public FunctionTypeInterface
    {
    public:
      using ArgsType = boost::callable_traits::args_t<Fn, ka::type_t>;
      using ReturnType = boost::callable_traits::return_type_t<Fn>;

      FunctionType()
      {
        _resultType = typeOf<ReturnType>();
        _argumentsType = anyfunction::argumentTypes(ArgsType{});
  #ifdef QITYPE_TRACK_FUNCTIONTYPE_INSTANCES
        detail::functionTypeTrack(qi::typeId<S>().name());
  #endif
      }

      void* call(void* storage, void** args, unsigned int argc) override
      {
        void *v = anyfunction::makeCall(
            *static_cast<Fn*>(ptrFromStorage(&storage)),
            std::vector(args, args + argc));
        // v is storage for type ReturnType we claimed we were
        // adapt return value if needed
        if (boost::is_pointer<ReturnType>::value &&
            (_resultType->kind() != TypeKind_Pointer ||
            static_cast<PointerTypeInterface*>(_resultType)->pointerKind() != PointerTypeInterface::Raw))
        {
          v = _resultType->clone(_resultType->initializeStorage(&v));
        }
        return v;
      }

      static FunctionType& instance()
      {
        // Intentionally leak a pointer, to avoid lifetime issues caused by
        // order of static initialization.
        static const auto INSTANCE = new FunctionType();
        return *INSTANCE;
      }

      _QI_BOUNCE_TYPE_METHODS(DefaultTypeImplMethods<Fn>);
    };

    // Pack arguments, call, wrap return value in AnyValue
    template<typename Fn>
    DynamicFunction makeDynamicFn(Fn fn)
    {
      if constexpr (std::is_member_pointer_v<Fn>) {
        using C = boost::callable_traits::class_of_t<Fn>;
        return [fn = std::move(fn)](const AnyReferenceVector& vargs) mutable {
          auto vargsIt = vargs.begin();
          QI_ASSERT(vargsIt != vargs.end());
          auto inst = static_cast<C*>(vargsIt->rawValue());
          if (!inst)
            qiLogWarning("qitype.AnyArgumentsBouncer") << "Null instance";
          AnyValueVector valueArgs(++vargsIt, vargs.end());
          return invokeIntoAnyReference(fn, inst, AnyArguments(valueArgs));
        };
      }
      else {
        return [fn = std::move(fn)](const AnyReferenceVector& vargs) mutable {
          AnyValueVector valueArgs(vargs.begin(), vargs.end());
          return invokeIntoAnyReference(fn, AnyArguments(valueArgs));
        };
      }
    }

    template<typename... Args>
    struct IsDynamicFnArgs : std::false_type {};

    template<typename Arg>
    struct IsDynamicFnArgs<Arg> : std::is_same<Arg, const AnyArguments&> {};

    template<typename... Args>
    struct IsDynamicMemberFnArgs : std::false_type {};

    template<typename C, typename... Args>
    struct IsDynamicMemberFnArgs<C, Args...> : IsDynamicFnArgs<Args...> {};

    // A dynamic function is a function or a member function that is invocable
    // with a sole `const AnyArguments&` argument. Functions like these are
    // wrapped in `DynamicFunction` objects.
    template <typename Fn>
    constexpr bool IsDynamicFn =
        std::disjunction_v<boost::callable_traits::args_t<Fn, IsDynamicFnArgs>,
                           std::conjunction<std::is_member_pointer<Fn>,
                                            boost::callable_traits::args_t<
                                                Fn, IsDynamicMemberFnArgs>>>;

    template<typename Fn>
    AnyFunction make(Fn fn)
    {
      if constexpr (IsDynamicFn<Fn>) {
        AnyFunction res = AnyFunction::fromDynamicFunction(makeDynamicFn(std::move(fn)));
        // The signature storage in GO will drop first argument, and bug if none is present
        const_cast<std::vector<TypeInterface*> &>(res.functionType()->argumentsType()).push_back(typeOf<AnyValue>());
        return res;
      }
      else {
        // The function is a function pointer or a member pointer.
        if constexpr (std::is_pointer_v<Fn> || std::is_member_pointer_v<Fn>) {
          auto& ftype = FunctionType<Fn>::instance();
          return AnyFunction(&ftype, ftype.clone(ftype.initializeStorage(&fn)));
        }
        // The function is a function object.
        else {
          // Lambdas are not handled well in our type system because they are copyable but not assignable.
          // Therefore we wrap any function object in a `boost::function` object.
          using FnType = boost::callable_traits::function_type_t<Fn>;
          using BoostFnType = boost::function<FnType>;
          auto& ftype = FunctionType<BoostFnType>::instance();
          return AnyFunction(&ftype, new BoostFnType(std::move(fn)));
        }
      }
    }

    template<typename R, typename F, typename B>
    AnyFunction make(boost::_bi::bind_t<R, F, B> v)
    {
      using CompatFnType = typename bind::boost_bind_function_type<boost::_bi::bind_t<R, F, B>>::type;
      boost::function<CompatFnType> f = v;
      return AnyFunction::from(std::move(f));
    }

    inline AnyFunction make(AnyFunction func)
    {
      return func;
    }
  }

  template<typename Fn>
  AnyFunction AnyFunction::from(Fn&& f)
  {
    return detail::anyfunction::make(std::forward<Fn>(f));
  }

  template<typename Fn, typename C>
  AnyFunction AnyFunction::from(Fn&& fn, C instance)
  {
    auto res = AnyFunction::from(std::forward<Fn>(fn));

    // Dynamic-cast instance to expected pointer type.
    using FnClass = boost::callable_traits::class_of_t<Fn>;
    FnClass* ptr = nullptr;
    if constexpr (std::is_pointer_v<C>) {
      ptr = dynamic_cast<FnClass*>(instance);
    } else {
      ptr = dynamic_cast<FnClass*>(&instance);
    }
    if (!ptr && instance)
      throw std::runtime_error("makeAnyFunction: failed to dynamic_cast bound value to expected type");
    res.prependArgument(reinterpret_cast<void*>(ptr));
    return res;
  }

KA_WARNING_POP()

}
#endif  // _QITYPE_DETAIL_ANYFUNCTIONFACTORY_HXX_
