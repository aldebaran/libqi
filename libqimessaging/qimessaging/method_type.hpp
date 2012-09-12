#ifndef _QIMESSAGING_METHODTYPE_HPP_
#define _QIMESSAGING_METHODTYPE_HPP_

#include <qimessaging/function_type.hpp>

namespace qi
{
  class QIMESSAGING_API MethodType: public virtual FunctionType
  {
  public:
    /// Call with all values of the correct type
    virtual void* call(void* method, void* object,
      const std::vector<void*>& args) = 0;
    /// Convert and call
    virtual Value call(void* method, Value object,
      const std::vector<Value>& args) = 0;
  };

  class QIMESSAGING_API MethodValue
  {
  public:
    Value call(Value object,
      const std::vector<Value> args)
    {
      return type->call(value, object, args);
    }
    std::string signature() const { return type->signature();}
    std::string sigreturn() const { return type->sigreturn();}
    ///@return equivalent function value
    FunctionValue toFunction();
    MethodType* type;
    void*       value;
  };

  template<typename T> MethodType* methodTypeOf();

  template<typename M>
  MethodValue makeMethodValue(const M& method);
}

#include <qimessaging/details/method_type.hxx>

#endif
