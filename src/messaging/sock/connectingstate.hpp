#pragma once
#ifndef _QI_SOCK_CONNECTINGSTATE_HPP
#define _QI_SOCK_CONNECTINGSTATE_HPP
#include <atomic>
#include <memory>
#include <string>
#include <boost/optional.hpp>
#include <boost/thread/synchronized_value.hpp>
#include <ka/memory.hpp>
#include <ka/functional.hpp>
#include <ka/utility.hpp>
#include <qi/future.hpp>
#include "connect.hpp"
#include "option.hpp"
#include "traits.hpp"

namespace qi
{
  namespace sock
  {
    /// Result of the connecting state.
    /// The socket is valid if there was no error and no disconnection was requested.
    ///
    /// Example:
    /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// // res is a ConnectingResult<N>
    /// const bool connectingFailed = hasError(res);
    /// if (connectingFailed || res.disconnectionRequested)
    /// {
    ///   connectedPromise.setError(connectingFailed
    ///     ? "Connect error: " + res.errorMessage
    ///     : "Abort: disconnection requested while connecting");
    ///
    ///   // res.socket is null but it is handled by enterDisconnectedState().
    ///   enterDisconnectedState(res.socket, res.disconnectedPromise);
    ///   return;
    /// }
    /// // res.socket is valid and can be used.
    /// // ...
    /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    ///
    /// Network N,
    /// With NetSslSocket S:
    ///   S is compatible with N
    template<typename N, typename S>
    struct ConnectingResult
    {
      std::string errorMessage;
      SocketPtr<S> socket;
      bool disconnectionRequested = false;
      Promise<void> disconnectedPromise;
      friend void setDisconnectionRequested(ConnectingResult& res, const Promise<void>& p)
      {
        res.disconnectionRequested = true;
        res.disconnectedPromise = p;
      }
      friend bool hasError(const ConnectingResult& x)
      {
        return !x.errorMessage.empty();
      }
    };

    /// Network N
    /// With NetSslSocket S:
    ///   S is compatible with N
    template<typename N, typename S>
    using SyncConnectingResult = boost::synchronized_value<ConnectingResult<N, S>>;

    /// Network N
    /// With NetSslSocket S:
    ///   S is compatible with N
    template<typename N, typename S>
    using SyncConnectingResultPtr = boost::shared_ptr<SyncConnectingResult<N, S>>;

    /// Connecting state of the socket.
    /// Connects to a URL and give back the created socket.
    ///
    /// Usage:
    /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// Connecting c{ioService, Url("tcps://1.2.3.4:9876"), IpV6Enabled{true},
    ///   HandshakeSide::client};
    /// auto socketPtr = c.complete().value();
    /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    ///
    /// The constructor immediately starts the connecting process, in the
    /// the same manner as std::thread. This way, there is no concurrency
    /// issue and no locking is done.
    /// If any error occurs, the `complete` future is set in error.
    ///
    /// # Lifetime considerations
    ///
    /// The object must be alive until the connecting process is complete.
    /// That is, you must not write:
    /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// Future<SocketPtr<S>> connect(IoService& io, Url url) {
    ///   return Connecting(io, url, IpV6Enabled{true}).complete();
    /// }
    /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// because the connecting process could exceed the ConnectSocket object lifetime.
    /// If the ConnectSocket dies before the end of the connecting process, the future
    /// is set in error.
    ///
    /// # Server-side
    ///
    /// It is also possible to give an already connected socket. In this case,
    /// if needed, only the SSL handshake is done. This is the typical use case
    /// for servers, because the `accept` pattern yields an already connected socket.
    ///
    /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// Connecting c{ioService, SslEnabled{true}, IpV6Enabled{true}, HandshakeSide::server};
    /// auto socketPtr = c.complete().value();
    /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    ///
    /// # Connecting steps
    ///
    /// From a more technical point of view, the different connecting steps
    /// are:
    /// - URL resolving
    /// - socket connecting
    /// - SSL handshake if needed.
    ///
    /// # Stopping the connection
    ///
    /// Example: stopping the connection
    /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// Promise<void> disconnectedPromise;
    /// connecting.stop(disconnectedPromise);
    /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// The promise will be passed as-is in the "complete" future. The purpose is to
    /// pass it to the next socket state, that will pass it again to the next state
    /// until it is finally set by the disconnected state.
    /// The stop is expected to happen almost immediately.
    ///
    /// # Technicalities of stopping
    ///
    /// The stop can happen in any step: resolving, connecting or handshaking.
    ///
    /// Stopping while resolving results in cancelling the resolver.
    /// Stopping while connecting or while handshaking results in the same
    /// action: closing the socket.
    ///
    /// These stopping actions are performed by continuations on a future. The stop is
    /// effectively triggered when the associated promise is set. This promise is
    /// owned by the `Connecting` object. A procedure is passed to the object that
    /// connects the socket to setup the stop, that is to set the continuations
    /// on the future.
    ///
    /// The promise must be destroyed before the object that connects the socket
    /// to avoid continuations to be called after the destruction of the resolver
    /// or the socket.
    ///
    /// Stopping kinematics:
    /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// -> `Connecting` object construction
    ///     | (1) creates
    ///     |
    ///     |           (3) calls when necessary
    ///     |           ------------------------
    ///     |          |                        |
    ///     v          v     (2) passed to      |
    /// setup-stop procedure ------------> connect-socket object
    ///        |
    ///        | (4) sets
    ///    -------------------------
    ///   |                         |
    ///   v                         v
    ///  cancelling resolver       closing socket
    ///  continuation              continuation (for connect & handshake steps)
    ///   ^                         ^
    ///   |                         |
    ///    -------------------------
    ///                        | (2') triggers
    ///           (1') sets    |
    /// -> stop() ---------> stop promise
    /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    ///
    /// Network N,
    /// With NetSslSocket S:
    ///   S is compatible with N
    template<typename N, typename S>
    struct Connecting
    {
      using Handshake = HandshakeSide<S>;

      struct Impl : std::enable_shared_from_this<Impl>
      {
        using std::enable_shared_from_this<Impl>::shared_from_this;
        Promise<SyncConnectingResultPtr<N, S>> _promiseComplete;
        SyncConnectingResultPtr<N, S> _result;
        ConnectSocketFuture<N, S> _connect;
        Promise<void> _promiseStop; // Must be declared after `_connect`, to be destroyed first
        std::atomic<bool> _stopping;

        void setContinuation()
        {
          auto self = shared_from_this();
          _connect.complete().then(
            [=](const Future<SocketPtr<S>>& fut) { // continuation
              {
                auto syncRes = self->_result->synchronize();
                if (fut.hasError())
                {
                  syncRes->errorMessage = fut.error();
                }
                else
                {
                  syncRes->socket = fut.value();
                }
              }
              self->_promiseComplete.setValue(self->_result);
            }
          );
        }
        Impl(IoService<N>& io)
          : _result{boost::make_shared<SyncConnectingResult<N, S>>()}
          , _connect{io}
          , _stopping{false}
        {
        }

        template<typename Proc0, typename Proc1 = ka::constant_function_t<void>>
        void start(const Url& url, Proc0&& makeSocket,
          IpV6Enabled ipV6, Handshake side, const boost::optional<Seconds>& tcpPingTimeout = {},
          Proc1 setupCancel = Proc1{})
        {
          setContinuation();
          _connect(url, ka::fwd<Proc0>(makeSocket), ipV6, side, tcpPingTimeout, setupCancel);
        }

        template<typename Proc = ka::constant_function_t<void>>
        void start(SslEnabled ssl, const SocketPtr<S>& s, Handshake side, Proc setupCancel = Proc{})
        {
          setContinuation();
          _connect(ssl, s, side, setupCancel);
        }
      };

      template<typename Proc0>
      Connecting(IoService<N>& io, const Url& url, Proc0&& makeSocket,
          IpV6Enabled ipV6, Handshake side, const boost::optional<Seconds>& tcpPingTimeout = {})
        : _impl(std::make_shared<Impl>(io))
      {
        using namespace ka;
        const auto implWeakPtr = weak_ptr(_impl);
        _impl->start(url, fwd<Proc0>(makeSocket), ipV6, side, tcpPingTimeout,
                     scope_lock_proc(makeSetupConnectionStop<N, S>(_impl->_promiseStop.future(),
                                                                 scope_lock_transfo(
                                                                 mutable_store(implWeakPtr)),
                                                                 StrandTransfo<N>{ &io }),
                                                                 mutable_store(implWeakPtr)));
      }
      Connecting(IoService<N>& io, SslEnabled ssl, const SocketPtr<S>& s, Handshake side)
        : _impl(std::make_shared<Impl>(io))
      {
        using namespace ka;
        const auto implWeakPtr = weak_ptr(_impl);
        _impl->start(ssl, s, side,
                     scope_lock_proc(makeSetupConnectionStop<N, S>(_impl->_promiseStop.future(),
                                                                 scope_lock_transfo(
                                                                 mutable_store(implWeakPtr)),
                                                                 StrandTransfo<N>{ &io }),
                                                                 mutable_store(implWeakPtr)));
      }
      Future<SyncConnectingResultPtr<N, S>> complete() const
      {
        return _impl->_promiseComplete.future();
      }
      bool stop(Promise<void> disconnectedPromise)
      {
        auto syncRes = _impl->_result->synchronize();
        const bool mustStop = tryRaiseAtomicFlag(_impl->_stopping);
        if (mustStop)
        {
          setDisconnectionRequested(*syncRes, disconnectedPromise);
          _impl->_promiseStop.setValue(nullptr); // triggers the stop
        }
        else
        {
          adaptFuture(syncRes->disconnectedPromise.future(), disconnectedPromise);
        }
        return mustStop;
      }
      static Future<void> connectError(const std::string& error)
      {
        return makeFutureError<void>(std::string("socket connection: ") + error);
      }
    private:
      std::shared_ptr<Impl> _impl;
    };
}} // namespace qi::sock

#endif // _QI_SOCK_CONNECTINGSTATE_HPP
