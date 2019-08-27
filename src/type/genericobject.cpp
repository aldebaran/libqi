#include <sstream>
#include <qi/anyobject.hpp>
#include <qi/log.hpp>

#include "metaobject_p.hpp"
#include <qi/os.hpp>

qiLogCategory("qitype.genericobject");

namespace qi
{

GenericObject::GenericObject(ObjectTypeInterface *type, void *value, const boost::optional<ObjectUid>& maybeUid)
  : type(type)
  , value(value)
  , uid(maybeUid ? *maybeUid : os::ptrUid(value))
{
}

GenericObject::~GenericObject() {}

const MetaObject &GenericObject::metaObject() {
  if (!type || !value) {
    static qi::MetaObject fail;
    qiLogWarning() << "Operating on invalid GenericObject..";
    return fail;
  }
  return type->metaObject(value);
}

Future<AnyReference> GenericObject::metaCallNoUnwrap(
    unsigned int method,
    const GenericFunctionParameters& params,
    const qi::MetaCallType callType,
    const Signature& returnSignature)
{
  QI_ASSERT(type && value && "invalid generic object");
  if (!type || !value)
    return qi::makeFutureError<AnyReference>("invalid generic object");

  try
  {
    return type->metaCall(value, AnyObject(shared_from_this()), method, params, callType, returnSignature);
  }
  catch (const std::exception &e)
  {
    return qi::makeFutureError<AnyReference>(e.what());
  }
  catch (...)
  {
    return qi::makeFutureError<AnyReference>("unknown error");
  }
}

qi::Future<AnyReference> GenericObject::metaCall(
    unsigned int method,
    const GenericFunctionParameters& params,
    MetaCallType callType,
    Signature returnSignature)
{
  Promise<AnyReference> unwrappedResult;
  auto result = metaCallNoUnwrap(method, params, callType, returnSignature);
  qi::adaptFutureUnwrap(result, unwrappedResult);
  return unwrappedResult.future();
}

qi::Future<AnyReference> GenericObject::metaCall(
    const std::string &nameWithOptionalSignature,
    const GenericFunctionParameters& args,
    MetaCallType callType,
    Signature returnSignature)
{
  int methodId = findMethod(nameWithOptionalSignature, args);
  if (methodId < 0) // in that case, the method ID is an error number
    return makeFutureError<AnyReference>(makeFindMethodErrorMessage(nameWithOptionalSignature, args, methodId));
  return metaCall(methodId, args, callType, returnSignature);
}

int GenericObject::findMethod(const std::string& nameWithOptionalSignature, const GenericFunctionParameters& args)
{
  return metaObject().findMethod(nameWithOptionalSignature, args);
}

std::string GenericObject::makeFindMethodErrorMessage(
    const std::string& nameWithOptionalSignature,
    const qi::GenericFunctionParameters& args,
    const int errorNo)
{
  auto resolvedSignature = args.signature(true).toString();
  return metaObject()._p->generateErrorString(
        nameWithOptionalSignature, resolvedSignature,
        metaObject().findCompatibleMethod(nameWithOptionalSignature),
        errorNo, false);
}

void GenericObject::metaPost(unsigned int event, const GenericFunctionParameters& args)
{
  if (!type || !value) {
    qiLogWarning() << "Operating on invalid GenericObject..";
    return;
  }
  type->metaPost(value, AnyObject(shared_from_this()), event, args);
}

/// Resolve signature and bounce
void GenericObject::metaPost(const std::string &nameWithOptionalSignature, const GenericFunctionParameters &in) {
  if (!value || !type) {
    qiLogWarning() << "Operating on invalid GenericObject..";
    return;
  }

  bool hasSig = (nameWithOptionalSignature.find(':') != std::string::npos);
  int eventId = metaObject().signalId(
    hasSig ? qi::signatureSplit(nameWithOptionalSignature)[1] : nameWithOptionalSignature);
  if (eventId < 0)
    eventId = findMethod(nameWithOptionalSignature, in);
  if (eventId < 0) {
    std::stringstream ss;
    std::string name = qi::signatureSplit(nameWithOptionalSignature)[1];
    ss << "Can't find method or signal: " << nameWithOptionalSignature << std::endl;
    ss << "  Method Candidate(s):" << std::endl;
    std::vector<MetaMethod>           mml2 = metaObject().findMethod(name);
    std::vector<MetaMethod>::const_iterator it2;
    for (it2 = mml2.begin(); it2 != mml2.end(); ++it2) {
      ss << "  " << it2->toString() << std::endl;
    }
    qiLogError() << ss.str();
    return;
  }
  metaPost(eventId, in);
}

//TODO: use functor.signature instead of nameWithSignature.
/// Resolve signature and bounce
qi::FutureSync<SignalLink> GenericObject::connect(const std::string &name, const SignalSubscriber& functor)
{
  if (!type || !value) {
    qiLogWarning() << "Operating on invalid GenericObject..";
    return qi::makeFutureError<SignalLink>("Operating on invalid GenericObject..");
  }
  int eventId = metaObject().signalId(name);

  if (eventId < 0) {
    std::ostringstream ss;
    ss << "signal \"" << name << "\" was not found";
    return qi::makeFutureError<SignalLink>(ss.str());
  }
  return connect(eventId, functor);
}

qi::FutureSync<SignalLink> GenericObject::connect(unsigned int event, const SignalSubscriber& sub)
{
  if (!type || !value) {
    qiLogWarning() << "Operating on invalid GenericObject..";
    return qi::makeFutureError<SignalLink>("Operating on invalid GenericObject..");
  }
  return type->connect(value, AnyObject(shared_from_this()), event, sub);
}

qi::FutureSync<void> GenericObject::disconnect(SignalLink linkId)
{
  if (!type || !value) {
    qiLogWarning() << "Operating on invalid GenericObject..";
    return qi::makeFutureError<void>("Operating on invalid GenericObject");
  }
  return type->disconnect(value, AnyObject(shared_from_this()), linkId);
}

qi::FutureSync<AnyValue> GenericObject::property(unsigned int id) {
  return type->property(value, AnyObject(shared_from_this()), id);
}

qi::FutureSync<void> GenericObject::setProperty(unsigned int id, const AnyValue& val) {
  return type->setProperty(value, AnyObject(shared_from_this()), id, val);
}

qi::FutureSync<SignalLink> GenericObject::connect(unsigned int signal, AnyObject target, unsigned int slot)
{
  return connect(signal, SignalSubscriber(target, slot));
}

/*
std::vector<SignalSubscriber> GenericObject::subscribers(int eventId) const
{
  std::vector<SignalSubscriber> res;
  if (!_p) {
    qiLogWarning() << "Operating on invalid GenericObject..";
    return res;
  }
  return _p->subscribers(eventId);
}*/

void GenericObject::post(const std::string& nameWithOptionalSignature,
                       qi::AutoAnyReference p1,
                       qi::AutoAnyReference p2,
                       qi::AutoAnyReference p3,
                       qi::AutoAnyReference p4,
                       qi::AutoAnyReference p5,
                       qi::AutoAnyReference p6,
                       qi::AutoAnyReference p7,
                       qi::AutoAnyReference p8)
{
  if (!type || !value) {
    qiLogWarning() << "Operating on invalid GenericObject..";
    return;
  }
  qi::AutoAnyReference* vals[8]= {&p1, &p2, &p3, &p4, &p5, &p6, &p7, &p8};
  std::vector<qi::AnyReference> params;
  for (unsigned i=0; i<8; ++i)
    if (vals[i]->type())
      params.push_back(*vals[i]);
  metaPost(nameWithOptionalSignature, GenericFunctionParameters(params));
}

}
