#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QIMESSAGING_FUNCTIONTYPE_HPP_
#define _QIMESSAGING_FUNCTIONTYPE_HPP_

#include <boost/function.hpp>

#include <qimessaging/type.hpp>

namespace qi {

  class QIMESSAGING_API CallableType
  {
  public:
    Type* resultType();
    const std::vector<Type*>& argumentsType();
    std::string signature() const;
    std::string sigreturn() const;
  protected:
    Type*              _resultType;
    std::vector<Type*> _argumentsType;
  };

  class QIMESSAGING_API FunctionType: public Type, public CallableType
  {
  public:
    virtual void* call(void* func, const std::vector<void*>& args) = 0;
    GenericValue call(void* func, const std::vector<GenericValue>& args);
  };

  template<typename T> FunctionType* makeFunctionType();

  /** Represents a generic callable function.
   * This class has value semantic.
  */
  class QIMESSAGING_API GenericFunction
  {
  public:
    GenericFunction();
    GenericValue call(const std::vector<GenericValue>& args);
    FunctionType* type;
    boost::function<void ()> value;
  };

  template<typename T> GenericFunction makeGenericFunction(boost::function<T> f);
  template<typename F> GenericFunction makeGenericFunction(F func);

  template<typename O, typename F> GenericFunction makeGenericFunction(O o, F f);

}

#include <qimessaging/details/functiontype.hxx>

#endif  // _QIMESSAGING_FUNCTIONTYPE_HPP_
