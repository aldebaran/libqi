/*
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <boost/make_shared.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/transform_view.hpp>
#include <boost/type_traits/remove_reference.hpp>
#include <boost/type_traits/add_pointer.hpp>
#include <boost/type_traits/remove_const.hpp>
#include <boost/function_types/function_arity.hpp>
#include <boost/function_types/function_type.hpp>
#include <boost/function_types/result_type.hpp>
#include <boost/function_types/parameter_types.hpp>

#include <boost/fusion/container/vector/convert.hpp>
#include <boost/fusion/include/as_vector.hpp>
#include <boost/fusion/algorithm/transformation/transform.hpp>
#include <boost/fusion/include/transform.hpp>
#include <boost/fusion/functional/invocation/invoke_function_object.hpp>
#include <boost/fusion/container/generation/make_vector.hpp>
#include <boost/fusion/include/make_vector.hpp>

#include <qimessaging/metafunction.hpp>

namespace qi {

MetaStorage::~MetaStorage()
{
  if (deleteOnDestruction)
  {
    for (unsigned i=0; i<parameterValues.size(); ++i)
      parameterValues[i].destroy();
  }
}

MetaFunctionParameters::MetaFunctionParameters()
: invalidateOnDestruction(false)
{
}

MetaFunctionParameters::~MetaFunctionParameters()
{
  // Invalidate storage if asked to and storage has something to invalidate
  if (invalidateOnDestruction && storage && !storage->parameterValues.empty())
  {
    // There can be only one invalidator, assert it
    assert(storage->hasInvalidator);
    storage->valid = false;
    storage->hasInvalidator = false;
    // Clear values as they are no longer valid.
    // buffer is still there, so if it was filled it will be used.
    storage->parameterValues.clear();
  }
}

MetaFunctionParameters::MetaFunctionParameters(const MetaFunctionParameters& b)
: invalidateOnDestruction(false)
{
  storage = b.storage;
}


MetaFunctionParameters& MetaFunctionParameters::operator=(const MetaFunctionParameters& b)
{
  storage = b.storage;
  invalidateOnDestruction = false;
  return *this;
}

MetaFunctionParameters MetaFunctionParameters::copy() const
{
  if (!storage)
    return MetaFunctionParameters();
  // If storage cannot be invalidated or has only a buffer , no need to copy
  if (!storage->hasInvalidator || storage->parameterValues.empty())
    return MetaFunctionParameters(*this);
  // Create a new storage, clone data in it
  if (!storage->valid && !storage->parameterBuffer.size())
    qiLogError("qi.meta") << "FunctionParameters were invalidated";
  qiLogDebug("qi.meta") << "Copying function parameters";
  MetaFunctionParameters res;
  res._initStorage();
  // We will copy values, mark for destruction
  res.storage->deleteOnDestruction = true;
  // Buffer is refcounted, no need to clone
  res.storage->parameterBuffer = storage->parameterBuffer;
  for (unsigned i=0; i<storage->parameterValues.size(); ++i)
    res.storage->parameterValues.push_back(storage->parameterValues[i].clone());
  return res;
}

MetaFunctionParameters::MetaFunctionParameters(Buffer b)
: invalidateOnDestruction(false)
{
  _initStorage();
  storage->parameterBuffer = b;
}

MetaFunctionParameters::MetaFunctionParameters(const std::vector<Value>& value, bool invalidate)
: invalidateOnDestruction(false)
{
  _initStorage();
  storage->parameterValues = value;
  invalidateOnDestruction = invalidate;
  storage->hasInvalidator = invalidate;
}

void MetaFunctionParameters::_initStorage()
{
  storage = boost::make_shared<MetaStorage>();
  storage->hasInvalidator = false;
  storage->valid = true;
  storage->deleteOnDestruction = false;
}

const std::vector<Value>& MetaFunctionParameters::getValues() const
{
  if (!storage || !storage->valid)
  {
    if (!storage)
      qiLogError("qi.meta") << "getValues() on uninitialized storage";
    else
      qiLogError("qi.meta") << "getValues() on invalidated storage";
    static std::vector<Value> dummy;
    return dummy;
  }
  if (storage->parameterValues.empty() && storage->parameterBuffer.size())
  {
    qiLogDebug("qi.meta") << "Generating values from serialization";
    convertToValues();
  }
  return storage->parameterValues;
}

const std::vector<Value> MetaFunctionParameters::getValues(const std::vector<Type*>& types) const
{
  if (!storage || !storage->valid)
  {
    if (!storage)
      qiLogError("qi.meta") << "getValues() on uninitialized storage";
    else
      qiLogError("qi.meta") << "getValues() on invalidated storage";
    static std::vector<Value> dummy;
    return dummy;
  }
  if (storage->parameterValues.empty() && storage->parameterBuffer.size())
  {
    qiLogDebug("qi.meta") << "Generating values from serialization";
    IDataStream in(storage->parameterBuffer);
    std::vector<Value> res;
    for (unsigned i=0; i<types.size(); ++i)
    {
      void* val = types[i]->deserialize(in);
      Value v;
      v.type = types[i];
      v.value = val;
      res.push_back(v);
    }
    return res;
  }
  else
  {
    if (storage->parameterValues.size() != types.size())
    {
      qiLogWarning("qi.functionparameters") << "Arity mismatch";
    }
    std::vector<Value> res;
    for (unsigned i=0; i<std::min(types.size(), storage->parameterValues.size())
      ; ++i)
    {

      Value c = storage->parameterValues[i].convert(*types[i]);
      qiLogDebug("qi.functionparameters") <<"convert "
       << storage->parameterValues[i].type->infoString() <<" "
       << c.type->infoString() << " " << c.value;
      res.push_back(c);
    }
    return res;
  }
}

const Buffer& MetaFunctionParameters::getBuffer() const
{
  if (!storage)
  {
    qiLogError("qi.meta") << "getBuffer() on uninitialized storage.";
    static Buffer b;
    return b;
  }
  if (!storage->parameterValues.empty() && !storage->parameterBuffer.size())
  {
    qiLogDebug("qi.meta") << "Generating serialization from values.";
    convertToBuffer();
  }
  return storage->parameterBuffer;
}

void MetaFunctionParameters::setSignature(const std::string& s)
{
  if (!storage)
    qiLogError("qi.meta") << "setSignature() on uninitialized storage";
  storage->signature = s;
}

void MetaFunctionParameters::convertToValues() const
{
  if (!storage || storage->signature.empty())
  {
    qiLogError("qi.meta") << "Signature data unavailable, cannot convert to values.";
    return;
  }
  if (!storage->parameterValues.empty() || !storage->parameterBuffer.size())
    return; // already done or nothing to do
  Signature s(storage->signature);
  qi::IDataStream in(storage->parameterBuffer);
  Signature::iterator i;
  for (i = s.begin(); i!= s.end(); ++i)
  {
    Type* m = Type::getCompatibleTypeWithSignature(*i);
    if (!m)
    {
      qiLogError("qi.meta") << "Unable to find MetaType compatible with " << *i;
      storage->parameterValues.push_back(Value());
    }
    else
    {
      void* val = m->deserialize(in);
      Value mv;
      mv.type = m;
      mv.value = val;
      storage->parameterValues.push_back(mv);
    }
  }
  storage->deleteOnDestruction = true;
  assert (!storage->hasInvalidator);
}

void MetaFunctionParameters::convertToBuffer() const
{
  if (!storage || storage->parameterBuffer.size() || storage->parameterValues.empty())
    return; // Already done or nothing to do
  qi::ODataStream out(storage->parameterBuffer);
  for (unsigned i=0; i<storage->parameterValues.size(); ++i)
    storage->parameterValues[i].serialize(out);
}

MetaFunctionParameters MetaFunctionParameters::convert(const Signature& sig) const
{
  std::vector<Value> dst;
  const std::vector<Value>& src = getValues();
  if (sig.size() != src.size())
  {
    qiLogError("qi.metafunction") << "convert: signature/params size mismatch"
      << sig.toString() << " " << sig.size() << " " << src.size();
    return MetaFunctionParameters();
  }
  Signature::iterator i = sig.begin();
  int idx = 0;
  for (;i != sig.end(); ++i,++idx)
  {
    Type* compatible = qi::Type::getCompatibleTypeWithSignature(*i);
    if (!compatible)
    {
      qiLogError("qi.metafunction") <<"convert: unknown type " << *i;
      compatible = src[idx].type;
    }
    dst.push_back(src[idx].convert(*compatible));
  }
  MetaFunctionParameters res(dst, false);
  res.storage->deleteOnDestruction = true;
  return res;
}

MetaFunctionParameters::Mode MetaFunctionParameters::getMode() const
{
  if (storage && !storage->parameterValues.empty())
    return Mode_Value;
  else
    return Mode_Buffer;
}


MetaFunctionResult::MetaFunctionResult()
{
}

MetaFunctionResult::MetaFunctionResult(Buffer buffer)
:MetaFunctionParameters(buffer)
{}

MetaFunctionResult::MetaFunctionResult(const Value& mv)
: MetaFunctionParameters(std::vector<Value>(&mv, &mv+1))
{
  storage->deleteOnDestruction = true;
}

Value MetaFunctionResult::getValue() const
{
  const std::vector<Value>& v = getValues();
  if (!v.empty())
    return v[0];
  else
    return Value();
}

}

QI_REGISTER_MAPPING("i", qi::int32_t);
QI_REGISTER_MAPPING("I", qi::uint32_t);
QI_REGISTER_MAPPING("d", double);
QI_REGISTER_MAPPING("f", float);
QI_REGISTER_MAPPING("[d]", std::vector<double>);
QI_REGISTER_MAPPING("[f]", std::vector<float>);
QI_REGISTER_MAPPING("[i]", std::vector<int>);
QI_REGISTER_MAPPING("s", std::string);
QI_REGISTER_MAPPING("[s]", std::vector<std::string>);
QI_REGISTER_MAPPING("r", qi::Buffer);
QI_REGISTER_MAPPING("m", qi::Value);
