#include <qi/log.hpp>

#include "authprovider_p.hpp"
#include "transportsocket.hpp"
#include "gwsdclient.hpp"

qiLogCategory("qigateway.sdclient");

namespace
{
template <typename T>
void promiseSetter(void* promisePtr, const qi::Message& msg, qi::TransportSocketPtr sdsocket)
{
  qi::Promise<T>* prom = static_cast<qi::Promise<T>*>(promisePtr);

  qi::AnyReference value;
  if (msg.type() != qi::Message::Type_Error)
  {
    qiLogVerbose() << "Setting promise for message " << msg.id();
    value = msg.value(qi::typeOf<T>()->signature(), sdsocket);
    prom->setValue(value.to<T>());
  }
  else
  {
    qiLogVerbose() << "Message " << msg.id() << " has an error.";
    static std::string sigerr = "m";
    value = msg.value(sigerr, sdsocket);
    prom->setError(value.toString());
  }
  value.destroy();
  delete prom;
}

template <>
void promiseSetter<void>(void* promisePtr, const qi::Message& msg, qi::TransportSocketPtr sdsocket)
{
  qi::Promise<void>* prom = static_cast<qi::Promise<void>*>(promisePtr);

  if (msg.type() != qi::Message::Type_Error)
    prom->setValue(0);
  else
  {
    static std::string sigerr("m");
    qi::AnyReference gvp = msg.value(sigerr, sdsocket).content();
    std::string err = gvp.asString();
    prom->setError(err);
    gvp.destroy();
  }
  delete prom;
}
}

namespace qi
{
GwSDClient::GwSDClient()
  : _messageReadyLink(0)
{
  connected.setCallType(MetaCallType_Direct);
}

GwSDClient::~GwSDClient()
{
  close();
}

FutureSync<void> GwSDClient::connect(const Url& url)
{
  _sdSocket = qi::makeTransportSocket(url.protocol());
  if (!_sdSocket)
    return qi::makeFutureError<void>(std::string("unrecognised protocol '") + url.protocol() +
                                     std::string("' in url '") + url.str() + "'");
  Promise<void> promise;
  Future<void> fut = _sdSocket->connect(url);
  fut.connect(&GwSDClient::onSocketConnected, this, _1, promise);
  return promise.future();
}

bool GwSDClient::isConnected() const
{
  return _sdSocket && _sdSocket->isConnected();
}

void GwSDClient::close()
{
  if (!isConnected())
    return;
  _sdSocket->messageReady.disconnect(_messageReadyLink);
  _sdSocket->disconnect();
}

Url GwSDClient::url() const
{
  return _sdSocket->url();
}

void GwSDClient::setClientAuthenticatorFactory(ClientAuthenticatorFactoryPtr authenticator)
{
  _authFactory = authenticator;
}

TransportSocketPtr GwSDClient::socket()
{
  return _sdSocket;
}

const MetaObject& GwSDClient::metaObject()
{
  return _metaObject;
}

static Message makeMessage(unsigned int action, AnyReference value = AnyReference(), const std::string& signature = "")
{
  Message msg;

  msg.setFunction(action);
  msg.setService(Message::Service_ServiceDirectory);
  msg.setType(Message::Type_Call);
  if (signature.length())
    msg.setValue(value, signature);
  return msg;
}

static Message makeMessage(unsigned int action, const AnyReferenceVector& values, const std::string& sig)
{
  Message msg;

  msg.setFunction(action);
  msg.setService(Message::Service_ServiceDirectory);
  msg.setType(Message::Type_Call);
  msg.setValues(values, sig);
  return msg;
}

Future<ServiceInfoVector> GwSDClient::services()
{
  Message msg = makeMessage(Message::ServiceDirectoryAction_Services);
  Promise<ServiceInfoVector>* prom = new Promise<ServiceInfoVector>;
  Future<ServiceInfoVector> fut = prom->future();
  unsigned int id = msg.id();

  {
    boost::mutex::scoped_lock lock(_promutex);
    _promises[id] = std::make_pair((void*)prom, &promiseSetter<ServiceInfoVector>);
  }
  _sdSocket->send(msg);
  qiLogVerbose() << "Keeping a promise for message " << id;
  return fut;
}

Future<ServiceInfo> GwSDClient::service(const std::string& name)
{
  Message msg = makeMessage(Message::ServiceDirectoryAction_Service, AnyReference::from(name), "s");
  Promise<ServiceInfo>* prom = new Promise<ServiceInfo>;
  Future<ServiceInfo> fut = prom->future();
  unsigned int id = msg.id();

  {
    boost::mutex::scoped_lock lock(_promutex);
    _promises[id] = std::make_pair((void*)prom, &promiseSetter<ServiceInfo>);
  }
  _sdSocket->send(msg);
  qiLogVerbose() << "Keeping a promise for message " << id;
  return fut;
}

Future<void> GwSDClient::unregisterService(unsigned int idx)
{
  Message msg = makeMessage(Message::ServiceDirectoryAction_UnregisterService, AnyReference::from(idx), "I");
  Promise<void>* prom = new Promise<void>;
  Future<void> fut = prom->future();
  unsigned int id = msg.id();

  {
    boost::mutex::scoped_lock lock(_promutex);
    _promises[id] = std::make_pair((void*)prom, &promiseSetter<void>);
  }
  _sdSocket->send(msg);
  qiLogVerbose() << "Keeping a promise for message " << id;
  return fut;
}

Future<std::string> GwSDClient::machineId()
{
  Message msg = makeMessage(Message::ServiceDirectoryAction_MachineId);
  Promise<std::string>* prom = new Promise<std::string>;
  Future<std::string> fut = prom->future();
  unsigned int id = msg.id();

  {
    boost::mutex::scoped_lock lock(_promutex);
    _promises[id] = std::make_pair((void*)prom, &promiseSetter<std::string>);
  }
  _sdSocket->send(msg);
  qiLogVerbose() << "Keeping a promise for message " << id;
  return fut;
}

Future<MetaObject> GwSDClient::fetchMetaObject()
{
  Message msg = makeMessage(Message::BoundObjectFunction_MetaObject,
                            AnyReference::from((unsigned int)Message::Service_ServiceDirectory),
                            "I");
  Promise<MetaObject>* prom = new Promise<MetaObject>;
  Future<MetaObject> fut = prom->future();
  unsigned int id = msg.id();

  {
    boost::mutex::scoped_lock lock(_promutex);
    _promises[id] = std::make_pair((void*)prom, &promiseSetter<MetaObject>);
  }
  _sdSocket->send(msg);
  qiLogVerbose() << "Keeping a promise for message " << id;
  return fut;
}

Future<SignalLink> GwSDClient::connectEvent(const std::string& eventName, SignalLink link)
{
  unsigned int sid = Message::Service_ServiceDirectory;
  unsigned int oid = _metaObject.signalId(eventName);
  AnyReferenceVector v;
  v.push_back(AnyReference::from(sid));
  v.push_back(AnyReference::from(oid));
  v.push_back(AnyReference::from(link));
  Message m = makeMessage(Message::BoundObjectFunction_RegisterEvent, v, "(IIL)");
  Promise<SignalLink>* prom = new Promise<SignalLink>;
  Future<SignalLink> fut = prom->future();
  unsigned int id = m.id();

  {
    boost::mutex::scoped_lock lock(_promutex);
    _promises[id] = std::make_pair((void*)prom, &promiseSetter<SignalLink>);
  }
  _sdSocket->send(m);
  qiLogVerbose() << "Keeping a promise for message " << id;
  return fut;
}

void GwSDClient::onEventConnected(Future<SignalLink> fut,
                                  Promise<void> prom,
                                  boost::shared_ptr<boost::mutex> mutex,
                                  boost::shared_ptr<int> initCount)
{
  if (fut.hasError())
  {
    std::string error = fut.error();
    qiLogError() << "onEventConnected:" << error;
    _sdSocket->disconnect();
    if (!prom.future().isFinished())
      prom.setError(error);
    return;
  }
  bool ready = false;
  {
    boost::mutex::scoped_lock lock(*mutex);
    *initCount += 1;
    ready = *initCount == 2;
  }
  if (ready)
  {
    prom.setValue(0);
    connected();
  }
}

void GwSDClient::onMetaObjectFetched(Future<MetaObject> fut, Promise<void> prom)
{
  if (fut.hasError())
  {
    std::string error = fut.error();
    qiLogError() << error;
    _sdSocket->disconnect();
    prom.setError(error);
    return;
  }
  _metaObject = fut.value();

  boost::shared_ptr<int> count = boost::make_shared<int>(0);
  boost::shared_ptr<boost::mutex> mutex = boost::make_shared<boost::mutex>();
  Future<SignalLink> futEv1 = connectEvent("serviceAdded", 0UL);
  futEv1.connect(&GwSDClient::onEventConnected, this, _1, prom, mutex, count);
  Future<SignalLink> futEv2 = connectEvent("serviceRemoved", 1UL);
  futEv2.connect(&GwSDClient::onEventConnected, this, _1, prom, mutex, count);
}

void GwSDClient::onAuthentication(const Message& msg,
                                  qi::Promise<void> prom,
                                  ClientAuthenticatorPtr authenticator,
                                  SignalSubscriberPtr old)
{
  static const std::string cmsig = typeOf<CapabilityMap>()->signature().toString();
  TransportSocketPtr sdSocket = _sdSocket;
  unsigned int function = msg.function();

  if (msg.type() == Message::Type_Error || msg.service() != Message::Service_Server ||
      function != Message::ServerFunction_Authenticate)
  {
    if (sdSocket)
      sdSocket->messageReady.disconnect(*old);
    std::stringstream error;
    if (msg.type() == Message::Type_Error)
      error << "Authentication failed: " << msg.value("s", sdSocket).to<std::string>();
    else
      error << "Expected a message for function #" << Message::ServerFunction_Authenticate
            << " (authentication), received a message for function " << msg.function();
    prom.setError(error.str());
    sdSocket->disconnect();
    return;
  }

  AnyReference cmref = msg.value(typeOf<CapabilityMap>()->signature(), sdSocket);
  CapabilityMap authData = cmref.to<CapabilityMap>();
  cmref.destroy();
  if (authData[AuthProvider::State_Key].to<AuthProvider::State>() == AuthProvider::State_Done)
  {
    if (sdSocket)
      sdSocket->messageReady.disconnect(*old);
    _messageReadyLink = sdSocket->messageReady.connect(&GwSDClient::onMessageReady, this, _1);
    qi::Future<MetaObject> future = fetchMetaObject();
    future.connect(&GwSDClient::onMetaObjectFetched, this, _1, prom);
    return;
  }

  CapabilityMap nextData = authenticator->processAuth(authData);
  Message authMsg;
  authMsg.setService(Message::Service_Server);
  authMsg.setType(Message::Type_Call);
  authMsg.setValue(nextData, cmsig);
  authMsg.setFunction(Message::ServerFunction_Authenticate);
  sdSocket->send(authMsg);
}

void GwSDClient::onSocketConnected(FutureSync<void> future, Promise<void> promise)
{
  if (future.hasError())
  {
    qiLogError() << future.error();
    promise.setError(future.error());
    TransportSocketPtr socket;
    std::swap(socket, _sdSocket);
    return;
  }
  _sdSocket->disconnected.connect(disconnected);
  ClientAuthenticatorPtr authenticator = _authFactory->newAuthenticator();
  CapabilityMap authCaps;
  {
    CapabilityMap tmp = authenticator->initialAuthData();
    for (CapabilityMap::iterator it = tmp.begin(), end = tmp.end(); it != end; ++it)
      authCaps[AuthProvider::UserAuthPrefix + it->first] = it->second;
  }
  SignalSubscriberPtr protocolSubscriber(new SignalSubscriber);
  *protocolSubscriber = _sdSocket->messageReady.connect(
      &GwSDClient::onAuthentication, this, _1, promise, authenticator, protocolSubscriber);

  CapabilityMap socketCaps = _sdSocket->localCapabilities();
  socketCaps.insert(authCaps.begin(), authCaps.end());

  Message msgCapabilities;
  msgCapabilities.setFunction(Message::ServerFunction_Authenticate);
  msgCapabilities.setService(Message::Service_Server);
  msgCapabilities.setType(Message::Type_Call);
  msgCapabilities.setValue(socketCaps, typeOf<CapabilityMap>()->signature());
  _sdSocket->send(msgCapabilities);
}

void GwSDClient::onMessageReady(const Message& msg)
{
  unsigned int id = msg.id();
  unsigned int type = msg.type();

  if (type == Message::Type_Event)
  {
    qiLogVerbose() << "Received a SD event";
    int event = msg.event();
    Signal<unsigned int, std::string>* sig = NULL;

    if (event == _metaObject.signalId("serviceAdded"))
      sig = &serviceAdded;
    else if (event == _metaObject.signalId("serviceRemoved"))
      sig = &serviceRemoved;
    else
    {
      qiLogVerbose() << "... in which we're not interested";
      return;
    }

    AnyReference values = msg.value("(Is)", _sdSocket);
    (*sig)(values[0].to<unsigned int>(), values[1].to<std::string>());
    values.destroy();
  }
  else
  {
    {
      boost::mutex::scoped_lock lock(_promutex);
      PromiseMap::iterator pit = _promises.find(id);

      if (pit == _promises.end())
      {
        qiLogVerbose() << "message " << id << " was intended for the GW, skipping.";
        return;
      }
      qiLogVerbose() << "Triggering promise for message " << id;
      pit->second.second(pit->second.first, msg, _sdSocket);
      _promises.erase(pit);
    }
  }
}
}
