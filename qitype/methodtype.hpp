#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QIMESSAGING_METHODTYPE_HPP_
#define _QIMESSAGING_METHODTYPE_HPP_

#include <boost/function.hpp>
#include <qimessaging/functiontype.hpp>

namespace qi
{
  class QIMESSAGING_API MethodType: public Type, public CallableType
  {
  public:
    /// Call with all values of the correct type
    virtual void* call(void* method, void* object,
      void** args, unsigned int nargs) = 0;
    /// Convert and call
    virtual GenericValue call(void* method, GenericValue object,
      const std::vector<GenericValue>& args) = 0;
    /// Return 'linearized' signature function type
    virtual FunctionType* toFunctionType() = 0;
  };

  /// Represents a generic member function. Has value semantic.
  class QIMESSAGING_API GenericMethod
  {
  public:
    GenericValue call(GenericValue object, const std::vector<GenericValue> args)
    {
      return type->call(&value, object, args);
    }
    std::string signature() const { return type->CallableType::signature();}
    std::string sigreturn() const { return type->CallableType::sigreturn();}

    GenericFunction toGenericFunction();
    MethodType* type;
    boost::function<void()> value;
  };

  template<typename T> MethodType* methodTypeOf();

  template<typename M>
  GenericMethod makeGenericMethod(const M& method);
}

#include <qimessaging/details/methodtype.hxx>

#endif  // _QIMESSAGING_METHODTYPE_HPP_
