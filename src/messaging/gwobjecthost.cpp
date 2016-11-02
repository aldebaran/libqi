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

    if (s->type() == Signature::Type_Object || s->type() == Signature::Type_Dynamic)
      return true;
    for (SignatureVector::const_iterator it = s->children().begin(), end = s->children().end(); it != end; ++it)
      queue.push(&(*it));
  }
  return false;
}

class MockObjectHost : public ObjectHost, public Trackable<MockObjectHost>
{
public:
  MockObjectHost(Message::Service s)
    : ObjectHost(s) {}
  ~MockObjectHost()
  {
    destroy();
  }

  unsigned int nextId() { return id_++; }
private:
  static unsigned int id_;
};

unsigned int MockObjectHost::id_ = 2;

void GwObjectHost::assignClientMessageObjectsGwIds(const Signature& signature, Message& msg, TransportSocketPtr sender, TransportSocketPtr destination)
{
  qiLogDebug() << "Assigning client message objects ids, signature: " << signature.toString();
  // if there's no chance of any object being in the call we're done.
  if (!hasObjectsSomewhere(signature))
    return;

  AnyReference callParameters = msg.value(signature, sender);

  // re-serialize the arguments so that the objects can receive a GW-specific objectId
  // ObjectHost uses a static int for its objectId so we're OK instantiating multiple
  // ones.
  Message forward;
  auto host = boost::make_shared<MockObjectHost>(Message::Service_Server);

  forward.setFlags(msg.flags());
  forward.setValue(callParameters, signature, host->weakPtr(), sender.get());
  msg.setBuffer(forward.buffer());

  // The message will store all the objects it serializes in the host.
  const ObjectHost::ObjectMap& objects = host->objects();
  std::map<GwObjectId, MetaObject> newObjectsMetaObjects;
  std::map<GwObjectId, ObjectInfo > newObjectsOrigin;
  std::vector<FullObjectAddress> newObjectFullAddresses;
  newObjectFullAddresses.reserve(objects.size());
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
    newObjectsOrigin[oid] = { sender, addr };
    newObjectFullAddresses.push_back(FullObjectAddress{sender, addr});
    newHostObjectBank[addr] = oid;
    // We set an empty transportsocket.
    // Otherwise when we destroy `passed` below, the remoteobject
    // will attempt to send back home a `terminate` message, which we don't want.
    // By setting a null socket the object will stay alive on the remote end.

    qiLogDebug() << "Message #" << msg.id() << ": object connection: {" << addr.service << "," << addr.object
                 << "} <=> {0," << oid << "}";
  }
  {
    boost::upgrade_lock<boost::shared_mutex> lock(_mutex);
    boost::upgrade_to_unique_lock<boost::shared_mutex> unique_lock(lock);
    _objectsMetaObjects.insert(newObjectsMetaObjects.begin(), newObjectsMetaObjects.end());
    _objectsOrigin.insert(newObjectsOrigin.begin(), newObjectsOrigin.end());

    // Remember who will keep the reference of the object, so that to unreference it when the socket is disconnected
    auto& objectOriginsForDestination = _objectOriginsPerDestination[destination];
    objectOriginsForDestination.insert(
          objectOriginsForDestination.end(),
          newObjectFullAddresses.begin(), newObjectFullAddresses.end());

    _hostObjectBank[sender].insert(newHostObjectBank.begin(), newHostObjectBank.end());
  }
  callParameters.destroy();
}

void GwObjectHost::harvestClientReplyOriginatingObjects(Message& msg, TransportSocketPtr sender, GwObjectId gwid, TransportSocketPtr destination)
{
  qiLogDebug() << "Harvesting client reply originating objects";
  Signature signature;
  {
    boost::shared_lock<boost::shared_mutex> lock(_mutex);
    const MetaMethod* method = _objectsMetaObjects[gwid].method(msg.function());
    if (!method)
      return;
    signature = method->returnSignature();
  }
  assignClientMessageObjectsGwIds(signature, msg, sender, destination);
}

void GwObjectHost::harvestClientCallOriginatingObjects(Message& msg, TransportSocketPtr sender, TransportSocketPtr destination)
{
  qiLogDebug() << "Harvesting client call originating objects";
  Signature signature;
  {
    boost::shared_lock<boost::shared_mutex> lock(_mutex);
    std::map<ObjectId, MetaObject>::iterator oit = _servicesMetaObjects[msg.service()].find(msg.object());
    if (oit == _servicesMetaObjects[msg.service()].end())
    {
      qiLogDebug() << "No metaobject for service " << msg.service() << ". Disconnected?";
      _unknownMetaObjectSockets[msg.service()][msg.object()] = sender;
      return;
    }
    const MetaMethod* method = oit->second.method(msg.function());
    if (!method)
      return;
    signature = method->parametersSignature();
  }
  assignClientMessageObjectsGwIds(signature, msg, sender, destination);
}

void GwObjectHost::harvestServiceOriginatingObjects(Message& msg, TransportSocketPtr sender, TransportSocketPtr destination)
{
  qiLogDebug() << "Harvesting objects from message for unregistered object";
  // To be able to find object occurrences in the message, guess the signature of remote member.
  // The message is for an unregistered object, so the object was eventually mirrored by
  // the gateway. This message's object address therefore corresponds to a local one.
  Signature signature;
  {
    boost::upgrade_lock<boost::shared_mutex> lock(_mutex);
    MetaObject* metaObject = NULL;
    const Signature& (MetaMethod::*signatureGetter)() const = NULL;
    if (msg.type() == Message::Type_Reply || msg.type() == Message::Type_Error)
    {
      auto sit = _servicesMetaObjects.find(msg.service());
      if (msg.function() == Message::BoundObjectFunction_MetaObject
        && sit != _servicesMetaObjects.end()
        && sit->second.find(msg.object()) == sit->second.end())
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
      // if a service does a CALL, it does so on a user-supplied object.
      metaObject = findMetaObject(msg.service(), msg.object());
      if (!metaObject)
      {
        throw std::runtime_error("Gateway Object Host: could not find object called by service");
      }
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
  auto host = boost::make_shared<MockObjectHost>(Message::Service_Server);
  Message dummy;

  // we don't want to pollute the original message and potentially change valid id
  // of contained objects, so we do it in an unrelated message.
  dummy.setValue(passed, signature, host->weakPtr(), &filler);

  const ObjectHost::ObjectMap& objects = host->objects();
  std::map<ObjectId, MetaObject> newServicesMetaObject;
  std::vector<FullObjectAddress> newObjectFullAddresses;
  newObjectFullAddresses.reserve(objects.size());
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

    ObjectAddress addr;
    addr.service = ro->service();
    addr.object = ro->object();
    newObjectFullAddresses.emplace_back(FullObjectAddress{ sender, addr });
  }
  {
    boost::upgrade_lock<boost::shared_mutex> lock(_mutex);
    boost::upgrade_to_unique_lock<boost::shared_mutex> unique_lock(lock);
    _servicesMetaObjects[msg.service()].insert(newServicesMetaObject.begin(), newServicesMetaObject.end());

    // Remember who will keep the reference of the object, so that to unreference it when the socket is disconnected
    auto& objectOriginsForDestination = _objectOriginsPerDestination[destination];
    objectOriginsForDestination.insert(
      objectOriginsForDestination.end(),
      newObjectFullAddresses.begin(), newObjectFullAddresses.end());
  }
  passed.destroy();
}

MetaObject* GwObjectHost::findMetaObject(ServiceId serviceId, ObjectId objectId)
{
  auto foundIt = _objectsMetaObjects.find(objectId);
  if (foundIt != _objectsMetaObjects.end())
  {
    return &foundIt->second;
  }

  auto foundServiceIt = _servicesMetaObjects.find(serviceId);
  if (foundServiceIt == _servicesMetaObjects.end())
  {
    return nullptr;
  }

  auto& metaObjectMap = foundServiceIt->second;

  auto foundObjectIt = metaObjectMap.find(objectId);
  if (foundObjectIt == metaObjectMap.end())
  {
    return nullptr;
  }

  return &foundObjectIt->second;
}

void GwObjectHost::harvestMessageObjects(Message& msg, TransportSocketPtr sender, TransportSocketPtr destination)
{
  qiLogDebug() << "Harvesting objects from message";
  if (msg.type() == Message::Type_Call || msg.type() == Message::Type_Post)
  {
    if (msg.service() == Message::Service_Server && msg.object() > Message::GenericObject_Main)
      harvestServiceOriginatingObjects(msg, sender, destination);
    else
      harvestClientCallOriginatingObjects(msg, sender, destination);
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
        harvestClientReplyOriginatingObjects(msg, sender, oit->second, destination);
        return;
      }
    }
    harvestServiceOriginatingObjects(msg, sender, destination);
  }
}

ObjectAddress GwObjectHost::getOriginalObjectAddress(const ObjectAddress& a)
{
  boost::shared_lock<boost::shared_mutex> lock(_mutex);
  QI_ASSERT(a.service == 0);
  auto info = objectSource(a);
  QI_ASSERT(info.socket);
  return info.address;
}

ObjectInfo GwObjectHost::objectSource(const ObjectAddress& address)
{
  {
    auto foundIt = _objectsOrigin.find(address.object);
    if (foundIt != _objectsOrigin.end())
      return foundIt->second;
  }

  {
    for (auto&& slot : _objectOriginsPerDestination)
    {
      auto& addressList = slot.second;
      auto findIt = std::find_if(addressList.begin(), addressList.end(), [&](const FullObjectAddress& fullAddress) {
        return fullAddress.localAddress.object == address.object;
      });
      if (findIt != addressList.end())
        return { findIt->socket.lock(), findIt->localAddress };
    }

  }

  return{};
}

static MetaObject extractReturnedMetaObject(const Message& msg, TransportSocketPtr);

void GwObjectHost::treatMessage(GwTransaction& t, TransportSocketPtr sender, TransportSocketPtr destination)
{
  qiLogDebug() << "treatMessage: " << t.content.address();
  Message& msg = t.content;

  if (msg.type() != Message::Type_Event)
    harvestMessageObjects(msg, sender, destination);

  if (msg.service() == Message::Service_Server && msg.object() > Message::GenericObject_Main &&
      (msg.type() == Message::Type_Call || msg.type() == Message::Type_Post))
  {
    boost::shared_lock<boost::shared_mutex> lock(_mutex);

    auto fullAddress = [&]() -> ObjectInfo {
      const auto objectIdToFind = msg.object();
      auto it = _objectsOrigin.find(objectIdToFind);
      if (it != _objectsOrigin.end())
        return { it->second.socket, it->second.address };

      for(auto&& slot : _objectOriginsPerDestination) // YOLO
      {
        auto& addressList = slot.second;
        auto it3 = std::find_if(addressList.begin(), addressList.end(), [&](const FullObjectAddress& a) {
          return a.localAddress.object == objectIdToFind;
        });
        if(it3 != addressList.end())
          return { it3->socket.lock(), it3->localAddress };
      }

      QI_ASSERT(false);
      return {};
    }();


    qiLogDebug() << "Changing content target from {" << t.content.service() << "," << t.content.object() << "} to {"
                 << fullAddress.address.service << "," << fullAddress.address.object << "}";
    t.content.setService(fullAddress.address.service);
    t.content.setObject(fullAddress.address.object);
    t.forceDestination(fullAddress.socket);
    qiLogDebug() << "Forcing destination: " << fullAddress.socket.get();
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
    auto& addr = _objectsOrigin[*it];
    Message terminateMessage;

    terminateMessage.setFunction(Message::BoundObjectFunction_Terminate);
    terminateMessage.setObject(addr.address.object);
    terminateMessage.setService(addr.address.service);
    terminateMessage.setType(Message::Type_Call);
    terminateMessage.setValue(AnyReference::from(addr.address.object), "I");
    addr.socket->send(terminateMessage);
  }
  objects.clear();
  _objectsUsedOnServices.erase(id);
}

void GwObjectHost::clientDisconnected(TransportSocketPtr socket)
{
  qiLogDebug() << "Processing client disconnection in gateway's object host, socket: " << (void*)socket.get();
  boost::upgrade_lock<boost::shared_mutex> lock(_mutex);

  // If the client had references to objects, kill them
  auto it = _objectOriginsPerDestination.find(socket);
  if (it != _objectOriginsPerDestination.end())
  {
    auto& fullObjectAddresses = it->second;
    for (auto& fullObjectAddress: fullObjectAddresses)
    {
      if (auto originSocket = fullObjectAddress.socket.lock())
      {
        auto& originObjectAddress = fullObjectAddress.localAddress;

        Message terminateMessage;
        terminateMessage.setFunction(Message::BoundObjectFunction_Terminate);
        terminateMessage.setObject(originObjectAddress.object);
        terminateMessage.setService(originObjectAddress.service);
        terminateMessage.setType(Message::Type_Call);
        terminateMessage.setValue(AnyReference::from(originObjectAddress.object), "I");
        originSocket->send(terminateMessage);
        qiLogDebug() << "Sending termination message to object host after remote disconnection: "
                     << terminateMessage;
      }
    }

    boost::upgrade_to_unique_lock<boost::shared_mutex> unique_lock(lock);
    _objectOriginsPerDestination.erase(it);
  }

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
    auto& serviceId = it->first.service;
    GwObjectId& gwObjectId = it->second;
    std::list<GwObjectId>& used = _objectsUsedOnServices[serviceId];
    allIds.push_back(gwObjectId);
    std::list<GwObjectId>::iterator uit = std::find(used.begin(), used.end(), gwObjectId);
    if (uit != used.end())
      used.erase(uit);
    if (used.size() == 0)
      _objectsUsedOnServices.erase(serviceId);
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

TransportSocketPtr GwObjectHost::findInUnknownMetaObjectSockets(ServiceId id)
{
  boost::shared_lock<boost::shared_mutex> lock(_mutex);
  auto it = _unknownMetaObjectSockets.find(id);
  if (it != end(_unknownMetaObjectSockets))
  {
    const auto& mapSocketByObjectId = it->second;
    if (!mapSocketByObjectId.empty())
    {
      return mapSocketByObjectId.begin()->second;
    }
  }
  return nullptr;
}

}
