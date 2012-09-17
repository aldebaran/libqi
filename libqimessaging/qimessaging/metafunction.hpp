/*
** Copyright (C) 2012 Aldebaran Robotics
*/


#pragma once

#ifndef _QI_MESSAGING_METAFUNCTION_HH_
#define _QI_MESSAGING_METAFUNCTION_HH_

#include <qimessaging/future.hpp>

#include <qimessaging/buffer.hpp>
#include <qimessaging/type.hpp>
#include <qimessaging/value.hpp>
#include <qimessaging/method_type.hpp>
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

/// Internal class that stores Values and/or Buffer in various forms.
class QIMESSAGING_API MetaStorage
{
public:
  /* Lifetime of Value is a bit tricky, as a Value can be on the stack,
   * or allocated.
   */
  ~MetaStorage();
  std::vector<Value> parameterValues;
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
  explicit MetaFunctionParameters(const std::vector<Value>& value, bool invalidateOnDestruction=false);
  /// Set from a buffer
  explicit MetaFunctionParameters(Buffer);

  /** Set signature associated with the value.
   *
   * The signature is required to convert buffer to values.
   */
  void setSignature(const std::string& sig);
  const std::vector<Value>& getValues() const;
  const Buffer& getBuffer() const;

  enum Mode
  {
    Mode_Value,
    Mode_Buffer
  };

  /// Return the mode available without conversion
  Mode getMode() const;
  /// Make a copy of storage if needed to garantee validity after async call.
  MetaFunctionParameters copy() const;

  /// Copy and convert storage to given signature
  MetaFunctionParameters convert(const qi::Signature& signature) const;

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
  MetaFunctionResult(const Value& value);
  MetaFunctionResult(Buffer);
  /// Return the value without copying it: Valid until storage goes.
  Value getValue() const;
};

MetaFunctionResult QIMESSAGING_API callFunction(FunctionValue function, const MetaFunctionParameters& params);
MetaFunctionResult QIMESSAGING_API callMethod(MethodValue function, Value instance, const MetaFunctionParameters& params);

/** Callable is the primary callback interface when 'dynamic' functions
 * can be needed: it takes the whole argument list as a MetaFunctionParameters.
 *
 */
typedef boost::function<MetaFunctionResult(const MetaFunctionParameters&)> MetaCallable;

template<typename T> MetaCallable makeCallable(T fun);
QIMESSAGING_API MetaCallable makeCallable(FunctionValue function);

}

#include "qimessaging/details/metafunction.hxx"


#endif
