/*
**  Copyright (C) 2012, 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#include <qi/anyobject.hpp>
#include <memory>

#include <ka/macro.hpp>
KA_WARNING_PUSH()
KA_WARNING_DISABLE(4355, )

qiLogCategory("qitype.object");

namespace qi
{

std::ptrdiff_t ObjectTypeInterface::inherits(TypeInterface* other)
{
  /* A registered class C can have to TypeInterface* around:
  * - TypeImpl<C*>
  * - The staticObjectTypeInterface that was created by the builder.
  * So assume that any of them can be in the parentTypes list.
  */
  if (this == other)
    return 0;
  const auto& parents = parentTypes();
  qiLogDebug() << infoString() <<" has " << parents.size() <<" parents";
  for (const auto& parentTypePair : parents)
  {
    const auto parentType = parentTypePair.first;
    const auto parentTypeOffset = parentTypePair.second;
    if (parentType->info() == other->info())
      return parentTypeOffset;
    ObjectTypeInterface* op = dynamic_cast<ObjectTypeInterface*>(parentType);
    if (op)
    {
      auto offset = op->inherits(other);
      if (offset != INHERITS_FAILED)
      {
        qiLogDebug() << "Inheritance offsets " << parentTypeOffset << " " << offset;
        return parentTypeOffset + offset;
      }
    }
    qiLogDebug() << parentType->infoString() << " does not match " << other->infoString()
    <<" " << ((op != 0) == (dynamic_cast<ObjectTypeInterface*>(other) != nullptr));
  }
  return INHERITS_FAILED;
}

namespace detail
{
  ProxyGeneratorMap& proxyGeneratorMap()
  {
    static ProxyGeneratorMap* map = nullptr;
    if (!map)
      map = new ProxyGeneratorMap();
    return *map;
  }
}

namespace {

static bool traceValidateSignature(const Signature& s)
{
  // Refuse to trace unknown (not serializable), object (too expensive), raw (possibly big)
  if (s.type() == Signature::Type_Unknown
      || s.type() == Signature::Type_Object
      || s.type() == Signature::Type_Raw
      || s.type() == Signature::Type_Pointer)
    return false;
  const SignatureVector& c = s.children();
  //return std::all_of(c.begin(), c.end(), traceValidateSignature);
  for (unsigned i=0; i<c.size(); ++i)
    if (!traceValidateSignature(c[i]))
      return false;
  return true;
}

// validate v for transmission to a trace signal
static const AnyValue& traceValidateValue(const AnyValue& v)
{
  static AnyValue fallback = AnyValue(AnyReference::from("**UNSERIALIZABLE**"));
  Signature s = v.signature(true);
  return traceValidateSignature(s)? v:fallback;
}

inline void call(qi::Promise<AnyReference>& out,
                 AnyObject context,
                 const GenericFunctionParameters& params,
                 unsigned int methodId,
                 AnyFunction& func,
                 unsigned int callerContext,
                 qi::os::timeval postTimestamp
                 )
{
  bool stats = context && context.isStatsEnabled();
  bool trace = context && context.isTraceEnabled();
  qi::AnyReference retref;
  int tid = 0; // trace call id, reused for result sending
  if (trace)
  {
    tid = context.asGenericObject()->_nextTraceId();
    qi::os::timeval tv(qi::SystemClock::now().time_since_epoch());
    AnyValueVector args;
    args.resize(params.size()-1);
    for (unsigned i=0; i<params.size()-1; ++i)
    {
      if (!params[i+1].type())
        args[i] = AnyValue::from("<??" ">");
      else
      {
        switch(params[i+1].type()->kind())
        {
        case TypeKind_Int:
        case TypeKind_String:
        case TypeKind_Float:
        case TypeKind_VarArgs:
        case TypeKind_List:
        case TypeKind_Map:
        case TypeKind_Tuple:
        case TypeKind_Dynamic:
        case TypeKind_Optional:
          args[i] = params[i+1];
          break;
        default:
          args[i] = AnyValue::from("<??" ">");
        }
      }
    }
    context.asGenericObject()->traceObject(EventTrace(
      tid, EventTrace::Event_Call, methodId, traceValidateValue(AnyValue::from(args)), tv,
      0,0, callerContext, qi::os::gettid(), postTimestamp));
  }

  qi::int64_t time = stats?qi::os::ustime():0;
  std::pair<int64_t, int64_t> cputime, cpuendtime;
  if (stats||trace)
     cputime = qi::os::cputime();

  bool success = false;
  try
  {
    qi::AnyReference ret;
    //the return value is destroyed by ServerResult in the future callback.
    ret = func.call(params);
    //copy the value for tracing later. (we want the tracing to happend after setValue
    if (trace)
      retref = ret.clone();
    //the reference, is dropped here... not cool man!
    out.setValue(ret);
    success = true;
  }
  catch(const std::exception& e)
  {
    success = false;
    out.setError(e.what());
  }
  catch(...)
  {
    success = false;
    out.setError("Unknown exception caught.");
  }

  if (stats||trace)
  {
    cpuendtime = qi::os::cputime();
    cpuendtime.first -= cputime.first;
    cpuendtime.second -= cputime.second;
  }

  if (stats)
    context.asGenericObject()->pushStats(methodId, (float)(qi::os::ustime() - time)/1e6f,
                       (float)cpuendtime.first / 1e6f,
                       (float)cpuendtime.second / 1e6f);


  if (trace)
  {
    qi::os::timeval tv(qi::SystemClock::now().time_since_epoch());
    AnyValue val;
    if (success)
      val = AnyValue(retref, false, true);
    else
      val = AnyValue::from(out.future().error());
    context.asGenericObject()->traceObject(EventTrace(tid,
      success?EventTrace::Event_Result:EventTrace::Event_Error,
      methodId, traceValidateValue(val), tv, cpuendtime.first, cpuendtime.second, callerContext, qi::os::gettid(), postTimestamp));
  }
}

class MFunctorCall
{
public:
  MFunctorCall(AnyFunction& func_, GenericFunctionParameters& params_,
     qi::Promise<AnyReference> out_, bool noCloneFirst_,
     AnyObject context_, unsigned int methodId_, unsigned int callerId_, qi::os::timeval postTimestamp_)
    : out(std::move(out_))
    , noCloneFirst(noCloneFirst_)
    , context(context_)
    , methodId(methodId_)
    , callerId(callerId_)
    , postTimestamp(postTimestamp_)
  {
    std::swap(this->func, func_);
    std::swap((AnyReferenceVector&) params_,
      (AnyReferenceVector&) this->params);
  }

  MFunctorCall(const MFunctorCall& b)
  {
    (*this) = b;
  }
  void operator = (const MFunctorCall& b)
  {
    // Implement move semantic on =
    std::swap( (AnyReferenceVector&) params,
      (AnyReferenceVector&) b.params);
    std::swap(func, const_cast<MFunctorCall&>(b).func);
    context = b.context;
    methodId = b.methodId;
    this->out = b.out;
    noCloneFirst = b.noCloneFirst;
    callerId = b.callerId;
    this->postTimestamp = b.postTimestamp;
  }
  void operator()()
  {
    call(out, context, params, methodId, func, callerId, postTimestamp);
    params.destroy(noCloneFirst);
  }
  qi::Promise<AnyReference> out;
  GenericFunctionParameters params;
  AnyFunction func;
  bool noCloneFirst;
  AnyObject context;
  unsigned int methodId;
  unsigned int callerId;
  qi::os::timeval postTimestamp;
};

}

qi::Future<AnyReference> metaCall(ExecutionContext* el,
  ObjectThreadingModel /*objectThreadingModel*/,
  MetaCallType methodThreadingModel,
  MetaCallType callType,
  AnyObject context,
  unsigned int methodId,
  AnyFunction func, const GenericFunctionParameters& params, bool noCloneFirst,
  unsigned int callerId,
  qi::os::timeval postTimestamp)
{
  // Implement rules described in header
  bool sync = false;
  if (methodThreadingModel != MetaCallType_Auto)
    sync = (methodThreadingModel == MetaCallType_Direct);
  else if (el)
    sync = el->isInThisContext();
  else if (callType != MetaCallType_Auto)
    sync = (callType == MetaCallType_Direct);

  if (!sync && !el)
    el = getEventLoop();

  qiLogDebug() << "metacall sync=" << sync << " el=" << el
               << " ct=" << callType;

  if (sync)
  {
    qi::Promise<AnyReference> out(FutureCallbackType_Sync);
    call(out, context, params, methodId, func,
         callerId ? callerId : qi::os::gettid(), postTimestamp);
    return out.future();
  }
  else
  {
    // If call is handled by our thread pool, we can safely switch the promise
    // to synchronous mode.
    qi::Promise<AnyReference> out;
    GenericFunctionParameters pCopy = params.copy(noCloneFirst);
    qi::Future<AnyReference> result = out.future();
    qi::os::timeval t(qi::SystemClock::now().time_since_epoch());
    el->post(MFunctorCall(func, pCopy, out, noCloneFirst, context,
                           methodId, callerId ? callerId : qi::os::gettid(), t));
    return result;
  }
}

}

KA_WARNING_POP()
