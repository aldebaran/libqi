#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QIMESSAGING_DETAILS_TYPE_HXX_
#define _QIMESSAGING_DETAILS_TYPE_HXX_

#include <qi/types.hpp>
#include <cstring>
#include <map>
#include <vector>
#include <list>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/transform_view.hpp>
#include <boost/type_traits/remove_reference.hpp>
#include <boost/type_traits/add_pointer.hpp>
#include <boost/function_types/parameter_types.hpp>
#include <boost/function_types/result_type.hpp>
#include <boost/function_types/function_type.hpp>
#include <boost/function_types/is_member_pointer.hpp>

#include <qimessaging/typespecialized.hpp>

/* This file contains the default-provided Type specialisations
 *
 */


namespace qi {
  // void
  template<> class TypeImpl<void>: public Type
  {
  public:
    TypeInfo info()
    {
      return typeid(void);
    }
    void* initializeStorage(void*) { return 0;}
    void* ptrFromStorage(void** ) { return 0;}
    void* clone(void*)                       { return 0;}
    void destroy(void* ptr)                  {}
    Kind kind() const { return Void;}
  };

  //reference

  template<typename T> class TypeImpl<T&>
      : public TypeImpl<T> {};

}



namespace qi  {

  namespace detail {
    template<typename T> inline Type* typeOfBackend()
    {
      Type* result = getType(typeid(T));
      if (!result)
      {

        static Type* defaultResult = 0;
        // Is this realy a problem?
        if (!defaultResult)
        qiLogVerbose("qi.meta") << "typeOf request for unregistered type "
          << typeid(T).name();
        if (!defaultResult)
          defaultResult = new TypeImpl<T>();
        result = defaultResult;
      }
      /*
      if (typeid(T).name() != result->infoString())
        qiLogDebug("qi.meta") << "typeOfBackend: type mismatch " << typeid(T).name() << " "
       << result <<" " << result->infoString();
       */
      return result;
    }

    template<typename T> struct TypeOfAdapter
    {
      typedef T type;
    };
    template<typename T> struct TypeOfAdapter<T&>
    {
      typedef typename TypeOfAdapter<T>::type type;
    };
    template<typename T> struct TypeOfAdapter<const T>
    {
      typedef typename TypeOfAdapter<T>::type type;
    };
    template<typename T> struct TypeOfAdapter<T*>
    {
      typedef typename boost::add_pointer<typename boost::remove_const<typename TypeOfAdapter<T>::type>::type>::type type;
    };
  }

  template<typename T> Type* typeOf()
  {
    return detail::typeOfBackend<typename detail::TypeOfAdapter<T>::type>();
  }

  inline Type::Kind Type::kind() const
  {
    return Unknown;
  }

  namespace detail {
    struct signature_function_arg_apply {
      signature_function_arg_apply(std::ostream* val)
        : val(*val)
      {}

      template<typename T> void operator()(T *x) {
        val << qi::typeOf<T>()->signature();
      }

      std::ostream &val;
    };

    template<typename T> struct RawFunctionSignature
    {
      static std::string makeSigreturn()
      {
        typedef typename boost::function_types::result_type<T>::type     ResultType;
        return typeOf<ResultType>()->signature();
      }
      static std::string makeSignature()
      {
        std::stringstream   signature;
        signature << '(';
        typedef typename boost::function_types::parameter_types<T>::type ArgsType;
        boost::mpl::for_each<
          boost::mpl::transform_view<ArgsType,
            boost::add_pointer<
              boost::remove_const<
                boost::remove_reference<boost::mpl::_1>
              >
            >
          >
        >
        (qi::detail::signature_function_arg_apply(&signature));
        signature << ')';
        return signature.str();
      }
    };
    template<typename T> struct MemberFunctionSignature
    {
      static std::string makeSigreturn()
      {
        typedef typename boost::function_types::result_type<T>::type     ResultType;
        return typeOf<ResultType>()->signature();
      }
      static std::string makeSignature()
      {
        // Reconstruct the boost::bind(instance, _1, _2...) signature
        typedef typename boost::function_types::result_type<T>::type     RetType;
        typedef typename boost::function_types::parameter_types<T>::type MemArgsType;
        typedef typename boost::mpl::pop_front< MemArgsType >::type                ArgsType;
        typedef typename boost::mpl::push_front<ArgsType, RetType>::type EffectiveType;
        typedef typename boost::function_types::function_type<EffectiveType>::type type;
        return RawFunctionSignature<type>::makeSignature();
      }
    };
    template<typename T> struct FunctionSignature
    {
      typedef typename  boost::mpl::if_<
        typename boost::function_types::is_member_pointer<T>,
        MemberFunctionSignature<T>,
        RawFunctionSignature<T>
        >::type Backend;
      static std::string signature()
      {
        static std::string result = Backend::makeSignature();
        return result;
      }
      static std::string sigreturn()
      {
        static std::string result = Backend::makeSigreturn();
        return result;
      }
    };
    template<typename T> struct FunctionSignature<boost::function<T> >
    : public FunctionSignature<T> {};

    template<typename T> inline
    std::string functionArgumentsSignature()
    {
      std::stringstream sigs;
      sigs << "(";
      typedef typename boost::function_types::parameter_types<T>::type ArgsType;
      boost::mpl::for_each<
      boost::mpl::transform_view<ArgsType,
      boost::add_pointer<
        boost::remove_const<
        boost::remove_reference<boost::mpl::_1> > > > > (qi::detail::signature_function_arg_apply(&sigs));
      sigs << ")";
      return sigs.str();
    }

    // Bouncer to DefaultAccess or DirectAccess based on type size
    template<typename T>
    class TypeImplMethodsBySize
    {
    public:
      typedef typename boost::mpl::if_c<
        sizeof(T) <= sizeof(void*),
        DefaultTypeImplMethods<T,
                        TypeByValue<T>
                        >,
        DefaultTypeImplMethods<T,
                        TypeByPointer<T>
                        >
                        >::type type;
    };
  }

  template<typename TypeDispatcher> TypeDispatcher& Type::dispatch(const TypeDispatcher & vv, void**storage)
  {
    TypeDispatcher& v = const_cast<TypeDispatcher&>(vv);
    switch(kind())
    {
    case Void:
      v.visitVoid(this);
      break;
    case Unknown:
      v.visitUnknown(this, storage);
      break;
    case Int:
      {
        TypeInt* tint = static_cast<TypeInt*>(this);
        /* Here we assume that '0' is represented by storage=0 in the byValue case.
        */
        v.visitInt(tint, (storage&&*storage)?tint->get(*storage):0, tint->isSigned(), tint->size());
        break;
      }
    case Float:
      {
        TypeFloat* tfloat = static_cast<TypeFloat*>(this);
        v.visitFloat(tfloat, (storage&&*storage)?tfloat->get(*storage):0, tfloat->size());
        break;
      }
    case String:
      {
        TypeString* tstring = static_cast<TypeString*>(this);
        v.visitString(tstring, *storage);
        break;
      }
    case List:
      {
        TypeList* tlist = static_cast<TypeList*>(this);
        v.visitList(GenericList(tlist, *storage));
        break;
      }
    case Map:
      {
        TypeMap * tlist = static_cast<TypeMap *>(this);
        v.visitMap(GenericMap(tlist, *storage));
        break;
      }
    case Object:
      {
        v.visitObject(GenericObject(static_cast<ObjectType*>(this), *storage));
        break;
      }
    case Pointer:
      {
        TypePointer* tpointer = static_cast<TypePointer*>(this);
        v.visitPointer(tpointer, *storage, (storage&&*storage)?tpointer->dereference(*storage):GenericValue());
        break;
      }
    case Tuple:
      {
      TypeTuple* ttuple = static_cast<TypeTuple*>(this);
      v.visitTuple(ttuple, *storage);
      break;
      }
    case Dynamic:
      {
      GenericValue gv;
      if (storage && *storage)
        gv = *(GenericValue*)ptrFromStorage(storage);
      v.visitDynamic(this, gv);
      }
    }
    return v;
  }
}

#include <qimessaging/details/typestring.hxx>
#include <qimessaging/details/typetuple.hxx>

#endif  // _QIMESSAGING_DETAILS_TYPE_HXX_
