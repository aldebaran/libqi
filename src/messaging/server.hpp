#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_SERVER_HPP_
#define _SRC_SERVER_HPP_

#include <boost/thread/recursive_mutex.hpp>
#include <boost/noncopyable.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include "boundobject.hpp"
#include "authprovider_p.hpp"

namespace qi
{

  namespace detail
  {
    namespace server
    {
      /// Wraps a socket with its server specific information, such as an authentication state and
      /// automatizes the connection/disconnection of a handler for server messages.
      class SocketInfo
      {
      public:
      // NonCopyable:
        SocketInfo(const SocketInfo&) = delete;
        SocketInfo& operator=(const SocketInfo&) = delete;

      // SocketInfo:
        SocketInfo(const MessageSocketPtr& socket,
                   SignalSubscriber onDisconnected,
                   AuthProviderPtr authProvider = {}) noexcept;
        ~SocketInfo();

        MessageSocketPtr socket() const noexcept { return _socket.lock(); }

        /// @invariant `(x.setAuthenticated(), x.isAuthenticated()) == true`
        void setAuthenticated() noexcept { _authenticated = true; }
        bool isAuthenticated() const noexcept{ return _authenticated; }

        /// @post `extractCapabilities().empty()`
        CapabilityMap extractCapabilities();

        /// @invariant `(x.setAuthProvider(p), x.authProvider()) == p`
        void setAuthProvider(AuthProviderPtr prov) noexcept { _authProvider = std::move(prov); }
        AuthProviderPtr authProvider() const noexcept { return _authProvider; }

        /// Adds a message handler for server specific messages.
        /// @throws A `std::logic_error` exception if an handler was already set.
        void setServerMessageHandler(MessageDispatcher::MessageHandler handler);

        struct Tracker : qi::Trackable<Tracker> { using Trackable::destroy; };
        Tracker tracker;

      private:
        const MessageSocketWeakPtr _socket;
        const qi::SignalLink _disconnected = qi::SignalBase::invalidSignalLink;
        AuthProviderPtr _authProvider;
        boost::optional<MessageDispatchConnection> _msgDispatchConnection;
        bool _authenticated = false;
        bool _capabilitiesTaken = false;
      };
      using SocketInfoPtr = std::unique_ptr<SocketInfo>;

      /// Helper class that simplifies the creation and destruction of bindings between service bound
      /// objects and sockets. It automatically binds each object with each socket that it stores.
      class BoundObjectSocketBinder
      {
        using SocketInfoList = std::vector<SocketInfoPtr>;
        using BoundObjectPtrMap = boost::container::flat_map<unsigned int, BoundObjectPtr>;
        using BoundObjectSocketBindingList = std::vector<detail::boundObject::SocketBinding>;

      public:
      // NonCopyable:
        BoundObjectSocketBinder(const BoundObjectSocketBinder&) = delete;
        BoundObjectSocketBinder& operator=(const BoundObjectSocketBinder&) = delete;

      // BoundObjectSocketBinder:
        BoundObjectSocketBinder() noexcept;

        /// A socket must be validated before it is bound to the objects. Usually a validation
        /// requires an authentication success.
        /// @throws A `std::logic_error` exception if the socket already exists in this binder.
        SocketInfo& addSocketPendingValidation(const MessageSocketPtr& socket,
                                               SignalSubscriber disconnectionHandlerAnyFunc);

        /// Removes a socket and unbinds it from the objects.
        /// @returns True if the socket existed in this binder and was succesfully removed.
        bool removeSocket(const MessageSocketPtr& socket) noexcept;

        /// Validates the socket and binds it to the objects.
        /// @returns The number of bindings that were created.
        std::size_t validateSocket(const MessageSocketPtr& socket) noexcept;

        struct ClearSocketsResult
        {
          std::size_t socketCount;
          std::size_t bindingCount;
          KA_GENERATE_FRIEND_REGULAR_OPS_2(ClearSocketsResult, socketCount, bindingCount)
        };

        /// Removes all sockets and consequently all existing bindings.
        /// @returns A result containing the number of socket that were removed and disconnected and
        /// the number of bindings destroyed.
        /// @post `socketsInfo().empty()`
        ClearSocketsResult clearSockets() noexcept;

        const SocketInfoList& socketsInfo() const { return _socketsInfo; }

        /// Adds a service bound object and binds it to the sockets.
        /// @returns True if an object did not exist already for this service id and the object was
        /// successfully added.
        bool addObject(unsigned int serviceId, BoundObjectPtr object) noexcept;

        /// Removes a service bound object and unbinds it from the sockets.
        /// @returns True if an object existed for this service id and it was successfully removed.
        bool removeObject(unsigned int serviceId) noexcept;

        const BoundObjectPtrMap& boundObjects() const { return _boundObjects; }

      private:
        /// @returns The number of bindings that were created.
        std::size_t bindObject(const BoundObjectPtr& object) noexcept;

        /// @returns The number of bindings that were created.
        std::size_t bindSocket(const MessageSocketPtr& socket) noexcept;

        /// @returns The number of bindings that were destroyed.
        std::size_t unbindSocket(const MessageSocketPtr& socket) noexcept;

        /// @returns The number of bindings that were destroyed.
        std::size_t unbindObject(const BoundObjectPtr& object) noexcept;

        BoundObjectPtrMap _boundObjects;
        SocketInfoList _socketsInfo;
        BoundObjectSocketBindingList _bindings;
      };
    }
  }

  /// Wraps a transport server and ensures the binding between service bound objects and message
  /// sockets. It handles messages of the protocol destinated to the "Server" of a endpoint, which
  /// includes the authentication of clients and exchange of protocol capabilities.
  ///
  /// The kinematics of server messages processing are as follows:
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///                      ╔═══════════════════════════╗
  ///                      ║ Socket received message { ║
  ///                      ║   service: Server(0)      ║
  ///                      ║   object: None(0)         ║
  ///                      ║ }                         ║
  ///                      ╚════════════╤══════════════╝
  ///                                   ▼
  ///              ╔════════════════════╧══════════════════════╗
  ///              ║ must_auth = Is authentication required ?  ║
  ///              ╟───────────────────────────────────────────╢
  ///              ║ is_auth = message.type == Call(1) and     ║
  ///              ║ message.action == Authenticate(8) ?       ║
  ///              ╚════════════════╤════════╤═════════════════╝
  ///               must_auth = yes ┤        ├ must_auth = no
  ///                               ╎        ╎
  ///          ┌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌┬╌╌╌┘        └╌╌╌┬╌╌╌╌╌╌╌╌╌╌╌╌╌╌┐
  ///          ╎                ╎                ╎              ╎
  ///          ├ is_auth        ├ is_auth        ├ is_auth      ├ is_auth
  ///          ╎ = yes          ╎ = no           ╎ = yes        ╎ = no
  ///          ╎                ╎                ╎              ╎
  ///          ╎                ▼                ╎              ▼
  ///          ╎    ╔═══════════╧═══════════╗    ╎  ╔═══════════╧═══════════╗
  ///          ╎    ║ Socket send message { ║    ╎  ║ Socket send message { ║
  ///          ╎    ║   type: Error(3)      ║    ╎  ║   type: Capability(6) ║
  ///          ╎    ║   service: Server(0)  ║    ╎  ║   service: Server(0)  ║
  ///          ╎    ║   object: None(0)     ║    ╎  ║   value: {            ║
  ///          ╎    ║ }                     ║    ╎  ║     <capabilities>    ║
  ///          ╎    ╚═══════════╤═══════════╝    ╎  ║   }                   ║
  ///          ▼                └╌╌╌╌╌╌╌╌╌╌╌╌╌╌┐ ╎  ║ }                     ║
  /// ╔════════╧══════════════════════╗        ╎ ╎  ╚═══════════╤═══════════╝
  /// ║  Is pair message.auth_user    ║        ╎ ╎              ╎
  /// ║ message.auth_password valid ? ║        ╎ ╎              ╎
  /// ╚════════╤═══════════════╤══════╝        ╎ ╎              ╎
  ///      yes ┤            no ┤               ╎ ╎              ╎
  ///          ╎               ▼               ╎ ╎              ╎
  ///          ╎  ╔════════════╧═════════════╗ ╎ ╎              ╎
  ///          ╎  ║ Socket send message {    ║ ╎ ╎              ╎
  ///          ╎  ║   type: Error(3)         ║ ╎ ╎              ╎
  ///          ╎  ║   service: Server(0)     ║ ╎ ╎              ╎
  ///          ╎  ║   object: None(0)        ║ ╎ ╎              ╎
  ///          ╎  ║   value: {               ║ ╎ ╎              ╎
  ///          ╎  ║     auth_state: Error(1) ║ ╎ ╎              ╎
  ///          ╎  ║   }                      ║ ╎ ╎              ╎
  ///          ╎  ║ }                        ║ ╎ ╎              ╎
  ///          ╎  ╚════════════╤═════════════╝ ╎ ╎              ╎
  ///          ╎               ├╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌┘ ╎              ╎
  ///          ╎               ▼                 ╎              ╎
  ///          ╎     ╔═════════╧═════════╗       ╎              ╎
  ///          ╎     ║ Disconnect socket ║       ╎              ╎
  ///          ╎     ╚═══════════════════╝       ╎              ╎
  ///          └╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌┬╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌┘              ╎
  ///                          ▼                                ╎
  ///             ╔════════════╧════════════╗                   ╎
  ///             ║ Socket send message {   ║                   ╎
  ///             ║   type: Reply(2)        ║                   ╎
  ///             ║   service: Server(0)    ║                   ╎
  ///             ║   object: None(0)       ║                   ╎
  ///             ║   value: {              ║                   ╎
  ///             ║     auth_state: Done(3) ║                   ╎
  ///             ║   }                     ║                   ╎
  ///             ║ }                       ║                   ╎
  ///             ╚════════════╤════════════╝                   ╎
  ///                          └╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌┬╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌┘
  ///                                           ▼
  ///                         ╔═════════════════╧════════════════╗
  ///                         ║ Ignore further socket messages { ║
  ///                         ║   service: Server(0)             ║
  ///                         ║   object: None(0)                ║
  ///                         ║ }                                ║
  ///                         ╚══════════════════════════════════╝
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  class Server : private boost::noncopyable
  {
  public:
    Server(ssl::ServerConfig sslConfig,
           boost::optional<AuthProviderFactoryPtr> authProviderFactory);
    ~Server();

    /// @see `TransportServer::listen`
    /// @post The server is not dying.
    qi::Future<void> listen(const qi::Url& address);

    /// @see `TransportServer::close`
    /// @post The server is dying.
    Future<void> close();

    /// Notifies the server that it's up again.
    /// @post The server is not dying.
    Future<bool> open();

    /// @returns True if the object is not null, an object did not exist already for this service id
    /// and the object was succesfully added.
    Future<bool> addObject(unsigned int serviceId, qi::AnyObject obj);
    Future<bool> addObject(unsigned int serviceId, qi::BoundObjectPtr obj);

    /// @returns True if an object existed for this service id and it was succesfully removed.
    Future<bool> removeObject(unsigned int idx);

    /// Adds an outgoing socket (a socket that was connected to a remote server endpoint instead of
    /// the usual listening server sockets). This allows service bounds objects to process messages
    /// coming from such sockets.
    /// @returns True if the socket started accepting messages.
    /// @throws
    ///   - A `std::invalid_argument` exception if the socket is null.
    Future<bool> addOutgoingSocket(MessageSocketPtr socket);

    /// @see `TransportServer::endpoints`
    Future<std::vector<qi::Uri>> endpoints() const;

    Future<void> setAuthProviderFactory(AuthProviderFactoryPtr factory);

  private:
    using SocketInfo = detail::server::SocketInfo;
    using BoundObjectSocketBinder = detail::server::BoundObjectSocketBinder;

    void closeImpl();

    /// Adds an incoming socket (a socket created from a client connecting to the server),
    /// optionally requiring authentication.
    /// @returns True if the socket started accepting messages.
    /// @pre The function is executed in the context of the strand.
    /// @throws
    ///   - A `std::invalid_argument` exception if the socket is null.
    bool addIncomingSocket(MessageSocketPtr socket);

    /// @pre The function is executed in the context of the strand.
    /// @returns true if a reply was sent to the client.
    bool handleServerMessage(const Message& msg, SocketInfo& socketInfo);

    /// @pre The function is executed in the context of the strand.
    /// @pre `_enforceAuth == true`
    /// @returns true if a reply was sent to the client.
    bool handleServerMessageAuth(const Message& msg, SocketInfo& socketInfo);

    /// @pre The function is executed in the context of the strand.
    /// @pre `_enforceAuth == false`
    /// @returns true if a reply was sent to the client.
    bool handleServerMessageNoAuth(const Message& msg, SocketInfo& socketInfo);

    /// @pre The function is executed in the context of the strand.
    /// @returns true if a reply was sent to the client.
    bool authenticateSocket(const qi::Message& msg, SocketInfo& socketInfo, Message reply);

    /// @pre The function is executed in the context of the strand.
    void finalizeSocketAuthentication(SocketInfo& socketInfo) noexcept;

    struct SendAuthErrorResult
    {
      bool errorSent;
      Future<void> disconnected;
      KA_GENERATE_FRIEND_REGULAR_OPS_2(SendAuthErrorResult, errorSent, disconnected)
    };

    static SendAuthErrorResult sendAuthError(std::string error,
                                             MessageSocket& socket,
                                             Message reply);

    static bool sendAuthReply(CapabilityMap authResult,
                              SocketInfo& socketInfo,
                              Message reply);

    static bool sendSuccessfulAuthReply(SocketInfo& socketInfo, Message reply);

    static bool sendCapabilitiesMessage(SocketInfo& socketInfo);

    struct RemoveSocketResult
    {
      bool removed;
      Future<void> disconnected;
      KA_GENERATE_FRIEND_REGULAR_OPS_2(RemoveSocketResult, removed, disconnected)
    };

    /// @see `BoundObjectSocketBinder::removeSocket`
    /// @pre `socket != nullptr`
    /// @pre The function is executed in the context of the strand.
    /// @returns A struct containing a boolean stating if the socket existed in the server and was
    /// succesfully removed and a future of the socket disconnection result.
    RemoveSocketResult removeSocket(const MessageSocketPtr& socket);

    // Mutable state of the object.
    struct State
    {
      AuthProviderFactoryPtr authProviderFactory;
      BoundObjectSocketBinder binder;
    };
    State _state;
    boost::shared_ptr<Strand> _strand = boost::make_shared<Strand>();

    struct Tracker : qi::Trackable<Tracker> { using Trackable::destroy; };
    Tracker _tracker;

    template<typename R>
    static Future<R> serverClosedError()
    {
      return makeFutureError<R>("The server is closed.");
    }

    template<
      typename Proc1,
      typename Result = ka::Decay<decltype(std::declval<Proc1>()())>,
      typename Proc2 = decltype(serverClosedError<Result>)>
      Future<Result> safeCall(Proc1&& proc, Proc2&& onFail = serverClosedError<Result>) const
    {
      if (auto ptr = boost::atomic_load(&_strand))
        return ptr->async(std::forward<Proc1>(proc));
      else
        return std::forward<Proc2>(onFail)();
    }

    /// @pre The function is executed in the context of the strand.
    /// @throws
    ///   - A `std::invalid_argument` exception if the socket is null.
    SocketInfo& addSocket(MessageSocketPtr socket);

  protected:
    const bool _enforceAuth;
    TransportServer _server;
    const qi::MetaCallType _defaultCallType;
  };

}


#endif  // _SRC_SERVER_HPP_
