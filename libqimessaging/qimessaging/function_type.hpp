#ifndef _QIMESSAGING_FUNCTIONTYPE_HPP_
#define _QIMESSAGING_FUNCTIONTYPE_HPP_

#include <qimessaging/type.hpp>

namespace qi {

  class QIMESSAGING_API FunctionType: public virtual Type
  {
  public:
    virtual void* call(void* func, const std::vector<void*>& args)=0;
    GenericValue call(void* func, const std::vector<GenericValue>& args);
    Type* resultType();
    const std::vector<Type*>& argumentsType();
    std::string signature() const;
    std::string sigreturn() const;
  protected:
    Type*              _resultType;
    std::vector<Type*> _argumentsType;
  };

  template<typename T> FunctionType* makeFunctionType();

  class QIMESSAGING_API FunctionValue
  {
  public:
    FunctionValue();
    FunctionValue(const GenericValue & v);
    GenericValue call(const std::vector<GenericValue>& args);
    FunctionType* type;
    void*         value;
  };

  template<typename T> FunctionValue makeFunctionValue(boost::function<T> f);
  template<typename F> FunctionValue makeFunctionValue(F func);

  template<typename O, typename F> FunctionValue makeFunctionValue(O o, F f);

}

#include <qimessaging/details/function_type.hxx>

#endif
