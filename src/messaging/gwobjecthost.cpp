#include <queue>

#include <qi/log.hpp>
#include <qi/type/metaobject.hpp>
#include <qi/signature.hpp>
#include <qi/jsoncodec.hpp>

#include "transportsocket.hpp"
#include "gwobjecthost.hpp"
#include "remoteobject_p.hpp"
#include "boundobject.hpp"
#include "gateway_p.hpp"

qiLogCategory("qigateway.objecthost");

namespace qi
{
GwObjectHost::~GwObjectHost()
{
}

static MetaObject extractReturnedMetaObject(const Message& msg, TransportSocketPtr);

static bool hasObjectsSomewhere(const Signature& sig)
{
  std::queue<const Signature*> queue;

  queue.push(&sig);
  while (queue.size())
  {
    const Signature* s = queue.front();
    queue.pop();

    if (s->type() == Signature::Type_Object)
      return true;
    for (SignatureVector::const_iterator it = s->children().begin(), end = s->children().end(); it != end; ++it)
      queue.push(&(*it));
  }
  return false;
}

class MockObjectHost : public ObjectHost
{
public:
  MockObjectHost(Message::Service s)
    : ObjectHost(s) {}
  unsigned int nextId() { return id_++; }
private:
  static unsigned int id_;
};

unsigned int MockObjectHost::id_ = 2;

void GwObjectHost::assignClientMessageObjectsGwIds(const Signature& signature, Message& msg, TransportSocketPtr sender)
{
  // if there's no chance of any object being in the call we're done.
  if (!hasObjectsSomewhere(signature))
    return;

  AnyReference callParameters = msg.value(signature, sender);

  // re-serialize the arguments so that the objects can receive a GW-specific objectId
  // ObjectHost uses a static int for its objectId so we're OK instantiating multiple
  // ones.
  Message forward;
  MockObjectHost host(Message::Service_Server);

  forward.setFlags(msg.flags());
  forward.setValue(callParameters, signature, &host, sender.get());
  msg.setBuffer(forward.buffer());

  // The message will store all the objects it serializes in the host.
  const ObjectHost::ObjectMap& objects = host.objects();
  std::map<GwObjectId, MetaObject> newObjectsMetaObjects;
  std::map<GwObjectId, std::pair<TransportSocketPtr, ObjectAddress> > newObjectsOrigin;
  std::map<ObjectAddress, GwObjectId> newHostObjectBank;
  for (ObjectHost::ObjectMap::const_iterator it = objects.begin(), end = objects.end(); it != end; ++it)
  {
    GwObjectId oid = it->first;
    ServiceBoundObject* sbo = static_cast<ServiceBoundObject*>(it->second.get());
    RemoteObject* ro = static_cast<RemoteObject*>(sbo->object().asGenericObject()->value);
    ObjectAddress addr;
    addr.service = ro->service();
    addr.object = ro->object();
    ro->setTransportSocket(TransportSocketPtr());

    newObjectsMetaObjects[oid] = ro->metaObject();
    newObjectsOrigin[oid] = std::make_pair(sender, addr);
    newHostObjectBank[addr] = oid;
    // We set an empty transportsocket.
    // Otherwise when we destroy `passed` below, the remoteobject
    // will attempt to send back home a `terminate` message, which we don't want.
    // By setting a null socket the object will stay alive on the remote end.

    qiLogDebug() << "Message " << msg.address() << ", Object connection: {" << addr.service << "," << addr.object
                 << "} <=> {0," << oid << "}";
  }
  {
    boost::upgrade_lock<boost::shared_mutex> lock(_mutex);
    boost::upgrade_to_unique_lock<boost::shared_mutex> unique_lock(lock);
    _objectsMetaObjects.insert(newObjectsMetaObjects.begin(), newObjectsMetaObjects.end());
    _objectsOrigin.insert(newObjectsOrigin.begin(), newObjectsOrigin.end());
    _hostObjectBank[sender].insert(newHostObjectBank.begin(), newHostObjectBank.end());
  }
  callParameters.destroy();
}

void GwObjectHost::harvestClientReplyOriginatingObjects(Message& msg, TransportSocketPtr sender, GwObjectId gwid)
{
  Signature signature;
  {
    boost::shared_lock<boost::shared_mutex> lock(_mutex);
    const MetaMethod* method = _objectsMetaObjects[gwid].method(msg.function());
    if (!method)
      return;
    signature = method->returnSignature();
  }
  assignClientMessageObjectsGwIds(signature, msg, sender);
}

void GwObjectHost::harvestClientCallOriginatingObjects(Message& msg, TransportSocketPtr sender)
{
  Signature signature;
  {
    boost::shared_lock<boost::shared_mutex> lock(_mutex);
    std::map<ObjectId, MetaObject>::iterator oit = _servicesMetaObjects[msg.service()].find(msg.object());
    if (oit == _servicesMetaObjects[msg.service()].end())
    {
      qiLogDebug() << "No metaobject for service " << msg.service() << ". Disconnected?";
      return;
    }
    const MetaMethod* method = oit->second.method(msg.function());
    if (!method)
      return;
    signature = method->parametersSignature();
  }
  assignClientMessageObjectsGwIds(signature, msg, sender);
}

void GwObjectHost::harvestServiceOriginatingObjects(Message& msg, TransportSocketPtr sender)
{
  Signature signature;
  {
    boost::upgrade_lock<boost::shared_mutex> lock(_mutex);
    MetaObject* metaObject = NULL;
    const Signature& (MetaMethod::*signatureGetter)() const = NULL;
    if (msg.type() == Message::Type_Reply || msg.type() == Message::Type_Error)
    {
      std::map<ServiceId, std::map<ObjectId, MetaObject> >::iterator sit = _servicesMetaObjects.find(msg.service());
      if (msg.function() == Message::BoundObjectFunction_MetaObject &&
          sit->second.find(msg.object()) == sit->second.end())
      {
        boost::upgrade_to_unique_lock<boost::shared_mutex> unique_lock(lock);
        _servicesMetaObjects[msg.service()][Message::GenericObject_Main] = extractReturnedMetaObject(msg, sender);
        return;
      }
      metaObject = &_servicesMetaObjects[msg.service()][msg.object()];
      signatureGetter = &MetaMethod::returnSignature;
    }
    else if (msg.type() == Message::Type_Call || msg.type() == Message::Type_Post)
    {
      // if a service does a CALL, he does so on a user-supplied object.
      std::map<GwObjectId, MetaObject>::iterator mit = _objectsMetaObjects.find(msg.object());
      assert(mit != _objectsMetaObjects.end());
      metaObject = &mit->second;
      signatureGetter = &MetaMethod::parametersSignature;
    }
    const MetaMethod* method = metaObject->method(msg.function());
    if (!method)
      return;
    signature = (method->*signatureGetter)();
  }
  if (!hasObjectsSomewhere(signature))
  {
    // no object can be here
    return;
  }

  AnyReference passed = msg.value(signature, sender);
  StreamContext filler;
  MockObjectHost host(Message::Service_Server);
  Message dummy;

  // we don't want to pollute the original message and potentially change valid id
  // of contained objects, so we do it in an unrelated message.
  dummy.setValue(passed, signature, &host, &filler);

  const ObjectHost::ObjectMap& objects = host.objects();
  std::map<ObjectId, MetaObject> newServicesMetaObject;
  for (ObjectHost::ObjectMap::const_iterator it = objects.begin(), end = objects.end(); it != end; ++it)
  {
    ServiceBoundObject* sbo = static_cast<ServiceBoundObject*>(it->second.get());
    RemoteObject* ro = static_cast<RemoteObject*>(sbo->object().asGenericObject()->value);

    // We set an empty transportsocket.
    // Otherwise when we destroy `passed` below, the remoteobject
    // will attempt to send back home a `terminate` message, which we don't want.
    // By setting a null socket the object will stay alive on the remote end.
    ro->setTransportSocket(TransportSocketPtr());
    newServicesMetaObject[ro->object()] = ro->metaObject();
  }
  {
    boost::upgrade_lock<boost::shared_mutex> lock(_mutex);
    boost::upgrade_to_unique_lock<boost::shared_mutex> unique_lock(lock);
    _servicesMetaObjects[msg.service()].insert(newServicesMetaObject.begin(), newServicesMetaObject.end());
  }
  passed.destroy();
}

void GwObjectHost::harvestMessageObjects(Message& msg, TransportSocketPtr sender)
{
  if (msg.type() == Message::Type_Call || msg.type() == Message::Type_Post)
  {
    if (msg.service() == Message::Service_Server && msg.object() > Message::GenericObject_Main)
      harvestServiceOriginatingObjects(msg, sender);
    else
      harvestClientCallOriginatingObjects(msg, sender);
  }
  else if (msg.type() == Message::Type_Reply)
  {
    std::map<TransportSocketPtr, std::map<ObjectAddress, GwObjectId> >::iterator sit = _hostObjectBank.find(sender);
    if (sit != _hostObjectBank.end())
    {
      ObjectAddress addr(msg.service(), msg.object());
      std::map<ObjectAddress, GwObjectId>::iterator oit = sit->second.find(addr);
      if (oit != sit->second.end())
      {
        harvestClientReplyOriginatingObjects(msg, sender, oit->second);
        return;
      }
    }
    harvestServiceOriginatingObjects(msg, sender);
  }
}

ObjectAddress GwObjectHost::getOriginalObjectAddress(const ObjectAddress& a)
{
  boost::shared_lock<boost::shared_mutex> lock(_mutex);
  assert(a.service == 0);
  auto it = _objectsOrigin.find(a.object);
  assert(it != _objectsOrigin.end());
  return it->second.second;
}

static MetaObject extractReturnedMetaObject(const Message& msg, TransportSocketPtr);

void GwObjectHost::treatMessage(GwTransaction& t, TransportSocketPtr sender)
{
  qiLogDebug() << "treatMessage: " << t.content.address();
  Message& msg = t.content;

  if (msg.type() != Message::Type_Event)
    harvestMessageObjects(msg, sender);

  if (msg.service() == Message::Service_Server && msg.object() > Message::GenericObject_Main &&
      (msg.type() == Message::Type_Call || msg.type() == Message::Type_Post))
  {
    boost::shared_lock<boost::shared_mutex> lock(_mutex);
    std::map<GwObjectId, std::pair<TransportSocketPtr, ObjectAddress> >::iterator it =
        _objectsOrigin.find(msg.object());
    assert(it != _objectsOrigin.end());
    qiLogDebug() << "Changing content target from {" << t.content.service() << "," << t.content.object() << "} to {"
                 << it->second.second.service << "," << it->second.second.object << "}";
    t.content.setService(it->second.second.service);
    t.content.setObject(it->second.second.object);
    t.forceDestination(it->second.first);
    qiLogDebug() << "Forcing destination: " << it->second.first.get();
  }
  else if (msg.type() == Message::Type_Reply || msg.type() == Message::Type_Error || msg.type() == Message::Type_Event)
  {
    boost::shared_lock<boost::shared_mutex> lock(_mutex);
    std::map<TransportSocketPtr, std::map<ObjectAddress, GwObjectId> >::iterator it = _hostObjectBank.find(sender);
    if (it != _hostObjectBank.end())
    {
      std::map<ObjectAddress, GwObjectId>::iterator oit =
          it->second.find(ObjectAddress(t.content.service(), t.content.object()));
      if (oit != it->second.end())
      {
        qiLogDebug() << "Changing content target from {" << t.content.service() << "," << t.content.object()
                     << "} to {" << Message::Service_Server << "," << oit->second << "}";
        t.content.setService(Message::Service_Server);
        t.content.setObject(oit->second);
      }
    }
  }
}

void GwObjectHost::serviceDisconnected(ServiceId id)
{
  boost::upgrade_lock<boost::shared_mutex> lock(_mutex);
  boost::upgrade_to_unique_lock<boost::shared_mutex> unique_lock(lock);
  _servicesMetaObjects.erase(id);
  std::map<ServiceId, std::list<GwObjectId> >::iterator findIt = _objectsUsedOnServices.find(id);
  if (findIt == _objectsUsedOnServices.end())
    return;
  std::list<GwObjectId>& objects = findIt->second;

  for (std::list<GwObjectId>::iterator it = objects.begin(), end = objects.end(); it != end; ++it)
  {
    std::pair<TransportSocketPtr, ObjectAddress>& addr = _objectsOrigin[*it];
    Message terminateMessage;

    terminateMessage.setFunction(Message::BoundObjectFunction_Terminate);
    terminateMessage.setObject(addr.second.object);
    terminateMessage.setService(addr.second.service);
    terminateMessage.setType(Message::Type_Call);
    terminateMessage.setValue(AnyReference::from(addr.second.object), "I");
    addr.first->send(terminateMessage);
  }
  objects.clear();
  _objectsUsedOnServices.erase(id);
}

void GwObjectHost::clientDisconnected(TransportSocketPtr socket)
{
  boost::upgrade_lock<boost::shared_mutex> lock(_mutex);
  // If the client had not registered any object, return.
  if (_hostObjectBank.find(socket) == _hostObjectBank.end())
    return;

  std::map<ObjectAddress, GwObjectId>& bank = _hostObjectBank[socket];
  if (bank.size() == 0)
    return;

  std::vector<GwObjectId> allIds;
  allIds.reserve(bank.size());
  boost::upgrade_to_unique_lock<boost::shared_mutex> unique_lock(lock);
  for (std::map<ObjectAddress, GwObjectId>::iterator it = bank.begin(), end = bank.end(); it != end; ++it)
  {
    ServiceId service = it->first.service;
    GwObjectId object = it->second;
    std::list<GwObjectId>& used = _objectsUsedOnServices[service];

    allIds.push_back(object);
    std::list<GwObjectId>::iterator uit = std::find(used.begin(), used.end(), object);
    if (uit != used.end())
      used.erase(uit);
    if (used.size() == 0)
      _objectsUsedOnServices.erase(service);
  }

  for (std::vector<GwObjectId>::iterator it = allIds.begin(), end = allIds.end(); it != end; ++it)
  {
    _objectsMetaObjects.erase(*it);
    _objectsOrigin.erase(*it);
  }
  _hostObjectBank.erase(socket);
}

static MetaObject extractReturnedMetaObject(const Message& msg, TransportSocketPtr sock)
{
  static const std::string retSig =
      "({I(Issss[(ss)<MetaMethodParameter,name,description>]s)<MetaMethod,uid,returnSignature,name,"
      "parametersSignature,description,parameters,returnDescription>}{I(Iss)<MetaSignal,uid,name,signature>}{I(Iss)<"
      "MetaProperty,uid,name,signature>}s)<MetaObject,methods,signals,properties,description>";

  AnyReference ref = msg.value((msg.flags() & Message::TypeFlag_DynamicPayload) ? "m" : retSig, sock);
  MetaObject obj = ref.to<MetaObject>();
  ref.destroy();
  return obj;
}
}
