#pragma once
#ifndef _QI_SOCK_CONNECTEDSTATE_HPP
#define _QI_SOCK_CONNECTEDSTATE_HPP
#include <memory>
#include <utility>
#include <boost/optional.hpp>
#include <ka/functional.hpp>
#include <ka/src.hpp>
#include <qi/atomic.hpp>
#include <qi/future.hpp>
#include <qi/url.hpp>
#include "src/messaging/message.hpp"
#include "common.hpp"
#include "receive.hpp"
#include "send.hpp"
#include "traits.hpp"

namespace qi
{
  namespace sock
  {

    /// The URL of the endpoint of the given socket.
    /// Precondition: The socket must be connected.
    ///
    /// NetSslSocket S
    template<typename S>
    Url remoteEndpoint(S& socket, TcpScheme scheme)
    {
      const auto endpoint = socket.lowest_layer().remote_endpoint();
      return url(endpoint, scheme);
    }

    /// Ouput of the connected state.
    ///
    /// It contains the socket, error information, and a promise to be set to
    /// indicate the end of the disconnection. This promise will typically be
    /// passed to the disconnecting state and finally set by the disconnected state.
    ///
    /// The only valid operation on the socket is to close it.
    ///
    /// The error message is valid only if there was an error.
    ///
    /// Example:
    /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// // res is a ConnectedResult<N>
    /// if (res.hasError)
    /// {
    ///   qiLogWarning() << "socket exited connected state with an error: "
    ///     << res.errorMessage;
    /// }
    /// close<N>(res.socket);
    /// res.disconnectedPromise.setValue(nullptr);
    /// // ...
    /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    ///
    /// Network N,
    /// With NetSslSocket S:
    ///   S is compatible with N
    template<typename N, typename S>
    struct ConnectedResult
    {
      SocketPtr<S> socket;

      /// If a disconnection was requested, this promise is related to the future
      /// returned to the caller. Setting it will inform the caller that the
      /// disconnection is complete (that is, the socket is in a disconnected state).
      Promise<void> disconnectedPromise;

      bool hasError;
      std::string errorMessage;

      ConnectedResult(SocketPtr<S> s = SocketPtr<S>{})
        : socket(s)
        , hasError(false)
      {
      }
    };

    /// Network N
    /// With NetSslSocket S:
    ///   S is compatible with N
    template<typename N, typename S>
    using SyncConnectedResult = boost::synchronized_value<ConnectedResult<N, S>>;

    /// Network N
    /// With NetSslSocket S:
    ///   S is compatible with N
    template<typename N, typename S>
    using SyncConnectedResultPtr = boost::shared_ptr<SyncConnectedResult<N, S>>;

    boost::optional<qi::int64_t> getSocketTimeWarnThresholdFromEnv();

    /// Connected state of the socket.
    /// Allow to send and receive messages.
    ///
    /// Received messages are exposed via a callback specified at construction.
    ///
    /// You can send messages continuously : they will be enqueued if needed.
    /// By default, enqueued messages are sent as soon as possible, but
    /// you can explicitly ask to stop the queue processing.
    ///
    /// You can request this state to stop, by optionally passing it a Promise
    /// to be set on disconnection.
    ///
    /// A `complete` future is set when the connected state is over.
    /// It contains the socket and a Promise to be set when disconnection is over.
    ///
    /// If an error occurs while sending or receiving, the `complete` future is
    /// set in error.
    ///
    /// Warning: You must call `stop()` before closing the socket, because
    /// closing the socket cause the send and receive handlers to be called with
    /// an 'abort' error, and we don't want to take it into account.
    ///
    /// Usage:
    /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// auto onReceive = [](ErrorCode e, const Message* msg) {
    ///   // `msg` is valid only if there is no error.
    ///   // Do something with `msg`.
    ///   return true; // Or return false to stop listening to message.
    /// };
    /// Connected<N> c{socketPtr, SslEnabled{true}, maxPayload, onReceive};
    /// // Get a Message msg;
    /// c.send(msg); // Move the message if you can, to avoid a copy.
    /// // Send more messages.
    /// Future<std::pair<SocketPtr<S>, Promise<void>>> futureComplete = c.complete();
    /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    ///
    /// Network N
    /// NetSslSocket N
    template<typename N, typename S>
    struct Connected
    {
    private:
      struct Impl : std::enable_shared_from_this<Impl>
      {
        using std::enable_shared_from_this<Impl>::shared_from_this;
        using ReadableMessage = typename SendMessageEnqueue<N, SocketPtr<S>>::ReadableMessage;

        boost::synchronized_value<Promise<SyncConnectedResultPtr<N, S>>> _completePromise;
        SyncConnectedResultPtr<N, S> _result;
        std::atomic<bool> _stopRequested;
        std::atomic<bool> _shuttingdown;
        ReceiveMessageContinuous<N> _receiveMsg;
        SendMessageEnqueue<N, SocketPtr<S>> _sendMsg;

        Impl(const SocketPtr<S>& socket);
        ~Impl();

        template<typename Proc>
        void start(SslEnabled, size_t maxPayload, Proc onReceive, qi::int64_t messageHandlingTimeoutInMus);

        template<typename Msg, typename Proc>
        void send(Msg msg, SslEnabled, Proc onSent);

        void stop(Promise<void> disconnectedPromise)
        {
          if (tryRaiseAtomicFlag(_stopRequested))
          {
            (*_result)->disconnectedPromise = disconnectedPromise;

            // The shutdown will cause any pending operation on the socket to fail.
            auto self = shared_from_this();
            ioServiceStranded([=] {
              self->_shuttingdown = true;
              auto socket = (*self->_result)->socket;
              socket->lowest_layer().cancel();
              socket->lowest_layer().shutdown(ShutdownMode<Lowest<S>>::shutdown_both);
            })();
          }
          else
          {
            // The disconnected promise has already been set.
            // Forward the result when we have it.
            adaptFuture((*_result)->disconnectedPromise.future(), disconnectedPromise);
          }
        }

        void setPromise(const sock::ErrorCode<N>&);

        SocketPtr<S>& socket()
        {
          return (*_result)->socket;
        }

        template<typename Proc>
        auto ioServiceStranded(Proc&& p)
          -> decltype(StrandTransfo<N>{std::declval<IoService<N>*>()}(std::forward<Proc>(p)))
        {
          return StrandTransfo<N>{ &N::getIoService(*socket()) }(std::forward<Proc>(p));
        }

        ka::data_bound_transfo_t<std::shared_ptr<Impl>> lifetimeTransfo()
        {
          return ka::data_bound_transfo(shared_from_this());
        }

        StrandTransfo<N> syncTransfo()
        {
          return { &N::getIoService(*socket()) };
        }
      };
      std::shared_ptr<Impl> _impl;
    public:
      /// If `onReceive` returns `false`, this stops the message receiving.
      ///
      /// Procedure<bool (ErrorCode<N>, const Message*)> Proc
      template<typename Proc>
      Connected(const SocketPtr<S>&, SslEnabled ssl, size_t maxPayload, const Proc& onReceive,
        qi::int64_t messageHandlingTimeoutInMus = getSocketTimeWarnThresholdFromEnv().value_or(0));

      /// If `onSent` returns false, the processing of enqueued messages stops.
      /// By default, we continue sending messages even if an error occurred.
      ///
      /// Procedure<bool (ErrorCode<N>, std::list<Message>::const_iterator)>
      template<typename Msg, typename Proc = ka::constant_function_t<bool>>
      void send(Msg&& msg, SslEnabled ssl, const Proc& onSent = {true})
      {
        return _impl->send(std::forward<Msg>(msg), ssl, onSent);
      }
      Future<SyncConnectedResultPtr<N, S>> complete() const
      {
        return _impl->_completePromise->future();
      }
      Url remoteEndpoint(TcpScheme scheme) const
      {
        return sock::remoteEndpoint(*_impl->socket(), scheme);
      }
      void stop(Promise<void> disconnectedPromise = Promise<void>{})
      {
        _impl->stop(disconnectedPromise);
      }
      template<typename Proc>
      auto ioServiceStranded(Proc&& p)
        -> decltype(_impl->ioServiceStranded(std::forward<Proc>(p)))
      {
        return _impl->ioServiceStranded(std::forward<Proc>(p));
      }
    };

    template<typename N, typename S>
    template<typename Proc>
    Connected<N, S>::Connected(const SocketPtr<S>& socket, SslEnabled ssl, size_t maxPayload,
        const Proc& onReceive, qi::int64_t messageHandlingTimeoutInMus)
      : _impl(std::make_shared<Impl>(socket))
    {
      _impl->start(ssl, maxPayload, onReceive, messageHandlingTimeoutInMus);
    }

    template<typename N, typename S>
    Connected<N, S>::Impl::Impl(const SocketPtr<S>& s)
      : _result{ boost::make_shared<SyncConnectedResult<N, S>>(ConnectedResult<N, S>{ s }) }
      , _stopRequested(false)
      , _shuttingdown(false)
      , _sendMsg{s}
    {
    }

    template<typename N, typename S>
    void Connected<N, S>::Impl::setPromise(const sock::ErrorCode<N>& error)
    {
      auto prom = _completePromise.synchronize();
      if (!prom->future().isRunning()) // promise already set
        return;
      const bool stopAsked = _stopRequested.load() && _shuttingdown.load();
      if (!stopAsked && error)
      {
        auto syncRes = _result->synchronize();
        syncRes->hasError = true;
        syncRes->errorMessage = error.message();
      }
      prom->setValue(_result);
    }

    template<typename N, typename S>
    template<typename Proc>
    void Connected<N, S>::Impl::start(SslEnabled ssl, size_t maxPayload, Proc onReceive,
        qi::int64_t /*messageHandlingTimeoutInMus*/)
    {
      auto self = shared_from_this();
      auto life = lifetimeTransfo();
      auto sync = syncTransfo();

      // Start receiving messages.
      // We preventively strand the first call.
      sync(life([=]() mutable {
        _receiveMsg(socket(), ssl, maxPayload,

          // This callback will be called when a message is received, or when an
          // error occurred. It returns a boolean to decide if message receiving
          // must continue.
          //
          // Warning: The message is only valid if there is no error. Also, it
          // is valid only until this callback ends. After that, the underlying
          // memory is typically reused, so the upper layer must not store the
          // message pointer (it can copy the message though).
          [=](sock::ErrorCode<N> e, Message* msg) mutable { // onReceived
            // If we're not shutting down, we inform the upper layer that we
            // received a message. In return, it decides if we must continue
            // receiving messages.
            const bool mustContinue = !_shuttingdown.load() && onReceive(e, msg);
            if (!mustContinue)
            {
              self->setPromise(e);
              return false; // We must not continue to receive messages.
            }
            return true; // Otherwise, we continue to receive messages.
          },
          life,
          sync
        );
      }))();
    }

    template<typename N, typename S>
    Connected<N, S>::Impl::~Impl()
    {
    }

    template<typename N, typename S>
    template<typename Msg, typename Proc>
    void Connected<N, S>::Impl::send(Msg msg, SslEnabled ssl, Proc onSent)
    {
      using SendMessage = decltype(_sendMsg);
      using ReadableMessage = typename SendMessage::ReadableMessage;
      auto self = shared_from_this();
      auto life = lifetimeTransfo();
      auto sync = syncTransfo();

      // We preventively strand the first call.
      sync(life(std::bind([=](Msg& msg) mutable {
        _sendMsg(std::move(msg), ssl,

          // This callback will be called when a message has been sent, or
          // when an error occurred.
          //
          // Warning: `ptrMsg` can be dereferenced to read the sent message.
          // This operation is only defined if no error occurred and only until
          // this callback ends. After that, the underlying memory is typically
          // freed, so the upper layer must not store the message pointer (it
          // can copy the message though).

          [=](const ErrorCode<N>& e, const ReadableMessage& ptrMsg) mutable {

            // If we're not shutting down, we inform the upper layer that we
            // sent a message. Then, the upper layer decides whether we should
            // continue sending messages or not by returning a boolean.
            const bool mustContinue = !_shuttingdown.load() && onSent(e, ptrMsg);
            if (!mustContinue)
            {
              self->setPromise(e);
              return false; // We must not continue to send messages.
            }
            return true; // Otherwise, we continue to send messages.
          },
          life,
          sync
        );
      }, std::move(msg))))();
    }
}} // namespace qi::sock

#endif // _QI_SOCK_CONNECTEDSTATE_HPP
