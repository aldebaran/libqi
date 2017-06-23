#pragma once
#ifndef _QI_SOCK_CONNECTEDSTATE_HPP
#define _QI_SOCK_CONNECTEDSTATE_HPP
#include <memory>
#include <utility>
#include <boost/optional.hpp>
#include <qi/atomic.hpp>
#include <qi/functional.hpp>
#include <qi/future.hpp>
#include <qi/messaging/sock/common.hpp>
#include <qi/messaging/sock/receive.hpp>
#include <qi/messaging/sock/send.hpp>
#include <qi/messaging/sock/traits.hpp>
#include "src/messaging/message.hpp"
#include <qi/url.hpp>

namespace qi
{
  namespace sock
  {

    /// The URL of the endpoint of the given socket.
    /// Precondition: The socket must be connected.
    ///
    /// NetSslSocket S
    template<typename S>
    Url remoteEndpoint(S& socket, bool /*ssl*/)
    {
      auto endpoint = socket.lowest_layer().remote_endpoint();
      // Forcing TCP is the legacy behavior.
      // TODO: Change this with `ssl ? "tcps" : "tcp"` when sure of the impact.
      return Url{
        endpoint.address().to_string(),
        "tcp",
        endpoint.port()};
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
    /// Network N
    template<typename N>
    struct ConnectedResult
    {
      SocketPtr<N> socket;

      /// If a disconnection was requested, this promise is related to the future
      /// returned to the caller. Setting it will inform the caller that the
      /// disconnection is complete (that is, the socket is in a disconnected state).
      Promise<void> disconnectedPromise;

      bool hasError;
      std::string errorMessage;

      ConnectedResult(SocketPtr<N> s = SocketPtr<N>{})
        : socket(s)
        , hasError(false)
      {
      }
    };

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
    /// Future<std::pair<SocketPtr<N>, Promise<void>>> futureComplete = c.complete();
    /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    ///
    /// Network N
    template<typename N>
    struct Connected
    {
    private:
      struct Impl : std::enable_shared_from_this<Impl>
      {
        using std::enable_shared_from_this<Impl>::shared_from_this;
        using ReadableMessage = typename SendMessageEnqueue<N, SocketPtr<N>>::ReadableMessage;

        boost::synchronized_value<Promise<ConnectedResult<N>>> _completePromise;
        ConnectedResult<N> _result;
        std::atomic<bool> _stopRequested;
        ReceiveMessageContinuous<N> _receiveMsg;
        SendMessageEnqueue<N, SocketPtr<N>> _sendMsg;
        boost::mutex _disconnectedPromiseMutex;

        Impl(const SocketPtr<N>& socket);
        ~Impl();

        template<typename Proc>
        void start(SslEnabled, size_t maxPayload, Proc onReceive, qi::int64_t messageHandlingTimeoutInMus);

        template<typename Msg, typename Proc>
        void send(Msg&& msg, SslEnabled, Proc onSent);

        void stop(Promise<void> disconnectedPromise)
        {
          boost::mutex::scoped_lock lock(_disconnectedPromiseMutex);
          if (tryRaiseAtomicFlag(_stopRequested))
          {
            _result.disconnectedPromise = disconnectedPromise;

            // The cancel will cause any pending operation on the socket to fail
            // with a "operation aborted" error.
            _result.socket->lowest_layer().cancel();
          }
          else
          {
            // The disconnected promise has already been set.
            // Forward the result when we have it.
            adaptFuture(_result.disconnectedPromise.future(), disconnectedPromise);
          }
        }

        void setPromise(const sock::ErrorCode<N>&, const Message*);

        SocketPtr<N>& socket()
        {
          return _result.socket;
        }
      };
      std::shared_ptr<Impl> _impl;
    public:
      /// If `onReceive` returns `false`, this stops the message receiving.
      ///
      /// Procedure<bool (ErrorCode<N>, const Message*)> Proc
      template<typename Proc>
      Connected(const SocketPtr<N>&, SslEnabled ssl, size_t maxPayload, const Proc& onReceive,
        qi::int64_t messageHandlingTimeoutInMus = getSocketTimeWarnThresholdFromEnv().value_or(0));

      /// If `onSent` returns false, the processing of enqueued messages stops.
      ///
      /// Procedure<bool (ErrorCode<N>, std::list<Message>::const_iterator)>
      template<typename Msg, typename Proc = NoOpProcedure<bool (ErrorCode<N>, std::list<Message>::const_iterator)>>
      void send(Msg&& msg, SslEnabled ssl, const Proc& onSent = {true})
      {
        return _impl->send(std::forward<Msg>(msg), ssl, onSent);
      }
      Future<ConnectedResult<N>> complete() const
      {
        return _impl->_completePromise->future();
      }
      Url remoteEndpoint(SslEnabled ssl) const
      {
        return sock::remoteEndpoint(*_impl->socket(), *ssl);
      }
      void stop(Promise<void> disconnectedPromise = Promise<void>{})
      {
        _impl->stop(disconnectedPromise);
      }
      template<typename Proc>
      auto ioServiceStranded(Proc&& p)
        -> decltype(StrandTransfo<N>{&_impl->socket()->get_io_service()}(std::forward<Proc>(p)))
      {
        return StrandTransfo<N>{&_impl->socket()->get_io_service()}(std::forward<Proc>(p));
      }
    };

    template<typename N>
    template<typename Proc>
    Connected<N>::Connected(const SocketPtr<N>& socket, SslEnabled ssl, size_t maxPayload,
        const Proc& onReceive, qi::int64_t messageHandlingTimeoutInMus)
      : _impl(std::make_shared<Impl>(socket))
    {
      _impl->start(ssl, maxPayload, onReceive, messageHandlingTimeoutInMus);
    }

    template<typename N>
    Connected<N>::Impl::Impl(const SocketPtr<N>& s)
      : _result{s}
      , _stopRequested(false)
      , _sendMsg{s}
    {
    }

    template<typename N>
    void Connected<N>::Impl::setPromise(const sock::ErrorCode<N>& error, const Message* msg)
    {
      auto prom = _completePromise.synchronize();
      if (!prom->future().isRunning()) // promise already set
        return;
      if (_stopRequested.load() && error == operationAborted<ErrorCode<N>>())
      {
        // This is not a real error: the user has asked the state to stop,
        // so we set the promise with a value instead of an error.
        prom->setValue(_result);
      }
      else if (error || !msg)
      {
        // A real error occurred.
        _result.hasError = true;
        _result.errorMessage = error.message();
        prom->setValue(_result);
      }
      else
      {
        // This is not an error: the user decided to stop after handling a message.
        prom->setValue(_result);
      }
    }

    template<typename N>
    template<typename Proc>
    void Connected<N>::Impl::start(SslEnabled ssl, size_t maxPayload, Proc onReceive,
        qi::int64_t messageHandlingTimeoutInMus)
    {
      auto self = shared_from_this();
      _receiveMsg(socket(), ssl, maxPayload,
        [=](sock::ErrorCode<N> e, const Message* msg) mutable { // onReceived
          const bool mustContinue = onReceive(e, msg);
          if (!mustContinue)
          {
            self->setPromise(e, msg);
            return false; // We must not continue to receive messages.
          }
          return true; // Otherwise, we continue to receive messages.
        },
        dataBoundTransfo(shared_from_this()), // lifetimeTransfo
        StrandTransfo<N>{&(*socket()).get_io_service()} // syncTransfo
      );
    }

    template<typename N>
    Connected<N>::Impl::~Impl()
    {
    }

    template<typename N>
    template<typename Msg, typename Proc>
    void Connected<N>::Impl::send(Msg&& msg, SslEnabled ssl, Proc onSent)
    {
      using SendMessage = decltype(_sendMsg);
      using ReadableMessage = typename SendMessage::ReadableMessage;
      auto self = shared_from_this();
      _sendMsg(std::forward<Msg>(msg), ssl,
        [=](const ErrorCode<N>& e, const ReadableMessage& ptrMsg) mutable { // onSent
          const bool mustContinue = onSent(e, ptrMsg);
          if (!mustContinue)
          {
            self->setPromise(e, &msg);
            return false; // We must not continue to send messages.
          }
          return true; // Otherwise, we continue to send messages.
        },
        dataBoundTransfo(shared_from_this()), // lifetimeTransfo
        StrandTransfo<N>{&(*socket()).get_io_service()} // syncTransfo
      );
    }
}} // namespace qi::sock

#endif // _QI_SOCK_CONNECTEDSTATE_HPP
