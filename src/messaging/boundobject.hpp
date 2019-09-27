#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_BOUNDOBJECT_HPP_
#define _SRC_BOUNDOBJECT_HPP_

#include <string>
#include <boost/thread/mutex.hpp>
#include <boost/signals2.hpp>
#include <boost/optional.hpp>
#include <boost/container/flat_map.hpp>
#include <qi/api.hpp>
#include <qi/session.hpp>
#include "transportserver.hpp"
#include <qi/atomic.hpp>
#include <qi/strand.hpp>

#include "objecthost.hpp"

using AtomicBoolptr = boost::shared_ptr<qi::Atomic<bool>>;
using AtomicIntPtr = boost::shared_ptr<qi::Atomic<int>>;

namespace qi
{

  class GenericObject;
  class ServiceDirectoryClient;
  class ServiceDirectory;

  // (service, linkId)
  struct RemoteSignalLink
  {
    RemoteSignalLink()
      : localSignalLinkId()
      , event(0)
    {}

    RemoteSignalLink(qi::Future<SignalLink> localSignalLinkId, unsigned int event)
    : localSignalLinkId(localSignalLinkId)
    , event(event) {}

    qi::Future<SignalLink> localSignalLinkId;
    unsigned int event;
  };

  /// This class represents the interface to a concrete object exposed to remote clients.
  ///
  /// Bound objects exist in 2 forms:
  ///   - Service bound objects are interfaces to the objects implementing services. They have a
  ///   special fixed object ID and they have no host / owner.
  ///   - Temporary bound objects are interfaces to objects created in the context of a socket
  ///   message and an host object. They are attached to the socket and are owned by their host.
  ///   They are owned by the remote that made the request which resulted in their creation and
  ///   in consequence it is destroyed once the remote releases it (terminates it).
  ///
  /// Service bound objects are created when objects in the same process are registered as services.
  ///
  /// Temporary bound objects are created when:
  ///   - An object is sent as the result of a incoming remote procedure call (those live in the
  /// process receiving the call, i.e. the server).
  ///   - An object is sent as a parameter of an outgoing remote procedure call (those live in the
  /// process sending the call, i.e. the client).
  class BoundObject
    : public ObjectHost
    , public boost::enable_shared_from_this<BoundObject>
    , boost::noncopyable
  {
    BoundObject(unsigned int serviceId,
                unsigned int objectId,
                qi::AnyObject obj,
                qi::MetaCallType mct = qi::MetaCallType_Queued,
                bool bindTerminate = false,
                boost::optional<boost::weak_ptr<ObjectHost>> owner = {});
  public:
    template<typename... Args>
    static BoundObjectPtr makePtr(Args&&... args)
    {
      return BoundObjectPtr(new BoundObject(std::forward<Args>(args)...));
    }

    ~BoundObject() override;

    unsigned int nextId() override { return ++_nextId; }

  public:
    //PUBLIC BOUND METHODS
    qi::Future<SignalLink> registerEvent(unsigned int serviceId, unsigned int eventId, SignalLink linkId);
    qi::Future<SignalLink> registerEventWithSignature(unsigned int serviceId, unsigned int eventId, SignalLink linkId, const std::string& signature);
    qi::Future<void> unregisterEvent(unsigned int serviceId, unsigned int eventId, SignalLink linkId);
    qi::MetaObject metaObject(unsigned int serviceId);
    void           terminate(unsigned int serviceId); //bound only in special cases
    qi::Future<AnyValue> property(const AnyValue& name);
    Future<void>   setProperty(const AnyValue& name, AnyValue value);
    std::vector<std::string> properties();
  public:
    /*
    * Returns the last socket that sent a message to this object.
    * Considering the volatility of this operation, Users of currentSocket()
    * must set _callType to Direct, otherwise behavior is undefined. Calling
    * currentSocket multiple times in a row in the same context should be
    * avoided: call it once and use the return value, unless you know what
    * you're doing.
    */
    inline qi::MessageSocketPtr currentSocket() const {
#ifndef NDEBUG
      if (_callType != MetaCallType_Direct)
        qiLogWarning("qimessaging.boundobject") << " currentSocket() used but callType is not direct";
#endif
      boost::recursive_mutex::scoped_lock lock(_mutex);
      return _currentSocket;
    }

    inline AnyObject object() { return _object;}
    unsigned int id() const { return _objectId; }

    void setOnSocketUnbound(boost::function<void (MessageSocketPtr)> cb)
    {
      _onSocketUnboundCallback = std::move(cb);
    }

  public:
    using MessageId = unsigned int;
    void cancelCall(MessageSocketPtr origSocket, const Message& cancelMessage, MessageId origMsgId);

    /// Registers the bound object as a handler of messages that might concern it (depending on the
    /// service and object ids) to the socket.
    /// @returns True if the binding was made and one didn't already exist for this socket.
    /// @invariant `b.bindToSocket(s) => b.unbindFromSocket(s)`
    bool bindToSocket(const MessageSocketPtr& socket) noexcept;

    /// Unregisters the bound object as a message handler from the socket and removes any entity
    /// created as a result of a message from this socket.
    /// @returns True if the object was succesfully unbound from the socket.
    bool unbindFromSocket(const MessageSocketPtr& socket) noexcept;

  private:
    using FutureMap =
      boost::container::flat_map<MessageId, std::pair<Future<AnyReference>, AtomicIntPtr>>;
    using CancelableMap = boost::container::flat_map<MessageSocketPtr, FutureMap>;
    // TODO: Use a synchronized_value instead.
    struct CancelableKit;
    using CancelableKitPtr = boost::shared_ptr<CancelableKit>;
    CancelableKitPtr _cancelables;
    using CancelableKitWeak = boost::weak_ptr<CancelableKit>;

    DispatchStatus onMessage(const qi::Message& msg, MessageSocketPtr socket);

    qi::AnyObject createBoundObjectType(BoundObject *self, bool bindTerminate = false);

    inline boost::weak_ptr<ObjectHost> _gethost()
    {
      return _owner ? *_owner : asHostWeakPtr();
    }

    boost::weak_ptr<ObjectHost> asHostWeakPtr()
    {
      // We guarantee this bound object is valid for the entirety of its tracker's lifetime, so
      // that its own lifetime can be bound to it.
      // Therefore it is valid to use the aliasing constructor of shared_ptr to transform a
      // shared_ptr of the tracker to a shared_ptr of its bound object.
      return weak_from_this();
    }

    static void _removeCachedFuture(CancelableKitWeak kit, MessageSocketPtr sock, MessageId id);
    static void serverResultAdapterNext(AnyReference val, Signature targetSignature,
                                        boost::weak_ptr<ObjectHost> host,
                                 MessageSocketPtr sock, const MessageAddress& replyAddr,
                                 const Signature& forcedReturnSignature, CancelableKitWeak kit);
    static void serverResultAdapter(Future<AnyReference> future, const Signature& targetSignature,
                                    boost::weak_ptr<ObjectHost> host,
                                    MessageSocketPtr sock, const MessageAddress& replyAddr,
                                    const Signature& forcedReturnSignature, CancelableKitWeak kit,
                                    AtomicIntPtr cancelRequested = AtomicIntPtr());

    // @returns The number of removed connections.
    std::size_t removeConnections(const MessageSocketPtr& socket) noexcept;

    // @returns The number of removed 'cancelable'.
    std::size_t removeCancelables(const MessageSocketPtr& socket) noexcept;

    // @returns The number of removed links.
    std::size_t removeLinks(const MessageSocketPtr& socket) noexcept;

    // remote link id -> local link id
    using ServiceSignalLinks = boost::container::flat_map<SignalLink, RemoteSignalLink>;
    using BySocketServiceSignalLinks =
      boost::container::flat_map<qi::MessageSocketPtr, ServiceSignalLinks>;

    // Event handling.
    BySocketServiceSignalLinks _links;

    // Locked when a `Call` message is received. It protects `_links`, `_object` and `_self`.
    // TODO: Use a synchronized_value instead.
    boost::recursive_mutex _callMutex;

    qi::MessageSocketPtr _currentSocket;

    using MessageDispatchConnectionList = std::vector<MessageDispatchConnection>;
    boost::synchronized_value<MessageDispatchConnectionList> _messageDispatchConnectionList;
    const unsigned int     _serviceId;
    const unsigned int     _objectId;
    const qi::AnyObject    _object;
    qi::AnyObject          _self;
    const qi::MetaCallType _callType;
    boost::optional<boost::weak_ptr<qi::ObjectHost>> _owner;
    // prevents parallel onMessage on self execution and protects the current socket
    // TODO: Use a synchronized_value instead.
    mutable boost::recursive_mutex           _mutex;
    boost::synchronized_value<boost::function<void (MessageSocketPtr)>> _onSocketUnboundCallback;

    static std::atomic<unsigned int> _nextId;
  };

  qi::BoundObjectPtr makeServiceBoundObjectPtr(unsigned int serviceId,
                                               qi::AnyObject object,
                                               qi::MetaCallType mct = qi::MetaCallType_Auto);

  namespace detail
  {
    namespace boundObject
    {
      /// Bound objects can receive messages originating from different sockets at a time (only
      /// service bound objects do). To represent this, we bind and unbind objects to sockets. This
      /// class represents a binding.
      ///
      /// It acts as a RAII helper to bind the object to the socket at construction and unbind it
      /// at destruction. Instances do not own their underlying socket.
      class SocketBinding
      {
      public:
      // MoveOnly:
        SocketBinding(const SocketBinding&) = delete;
        SocketBinding& operator=(const SocketBinding&) = delete;

        SocketBinding(SocketBinding&&);
        SocketBinding& operator=(SocketBinding&&);

      // SocketBinding:
        /// @pre `object != nullptr && socket != nullptr`
        SocketBinding(BoundObjectPtr object, MessageSocketPtr socket) noexcept;
        ~SocketBinding();

        BoundObjectPtr object() const noexcept { return _object; }

        MessageSocketPtr socket() const noexcept { return _socket.lock(); }

      private:
        void reset() noexcept;

        BoundObjectPtr _object;
        MessageSocketWeakPtr _socket;
      };
    }
  }
}

#endif  // _SRC_BOUNDOBJECT_HPP_
