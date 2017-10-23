#pragma once
#ifndef _QI_SOCK_RECEIVE_HPP
#define _QI_SOCK_RECEIVE_HPP
#include <qi/messaging/sock/concept.hpp>
#include <qi/messaging/sock/traits.hpp>
#include <qi/messaging/sock/option.hpp>
#include <qi/messaging/sock/error.hpp>
#include <qi/messaging/sock/common.hpp>
#include "src/messaging/message.hpp"
#include <qi/trackable.hpp>
#include <qi/log.hpp>
#include <qi/macroregular.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/optional.hpp>

/// @file
/// Contains functions and types related to message reception on a socket.

namespace qi { namespace sock {
  /// Receive a message through the socket and call the handler when the
  /// operation is complete, successfully or not.
  ///
  /// The handler is called with an error code and a "pointer" to the received
  /// message. This "pointer" can only be dereferenced if there is no error.
  ///
  /// If the handler returns a valid message "pointer" (typically a
  /// boost::optional<Message*>), the function immediately waits again for a
  /// new message arrival.
  /// The received message will be stored inside the "pointer".
  ///
  /// Precondition: The message referred to by `ptrMsg` must be valid until the
  ///   handler has been called.
  ///
  /// Precondition: This function must not be called while a message is already
  ///   being sent. It is possible to call it again only once the handler has
  ///   been called.
  ///
  /// Example: receiving messages until an error occurs
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// // Create a Message msg.
  /// receiveMessage<N>(socket, &msg, SslEnabled{false}, maxPayload,
  ///   [=](Error e, const Message* msgPtr) -> boost::optional<Message*> {
  ///     if (e) {
  ///       // treat error
  ///       return {}; // Stop receiving messages.
  ///     }
  ///     // use msgPtr
  ///     return {msgPtr}; // We return the memory to use to receive the next message.
  ///   });
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// Example: receiving one message, synchronously
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// // Create a Message msg.
  /// Promise<Message> promise;
  /// receiveMessage<N>(socket, &msg, SslEnabled{false}, maxPayload,
  ///   [=](Error e, const Message* msgPtr) mutable -> boost::optional<Message*> {
  ///     if (e) {
  ///       promise.setError(e.message());
  ///       return {};
  ///     }
  ///     promise.setValue(*msgPtr);
  ///     return {}; // We don't want to receive another message.
  ///   });
  ///   auto msg = promise.future().value();
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// Network N,
  /// Mutable<SslSocket<N>> S,
  /// Mutable<Message> M,
  /// Procedure<Optional<M> (ErrorCode<N>, M)> Proc,
  /// Transformation<Procedure> F0,
  /// Transformation<Procedure<void (Args...)>> F1
  template<typename N, typename S, typename M, typename Proc, typename F0 = IdTransfo, typename F1 = IdTransfo>
  void receiveMessage(const S& socket, M ptrMsg, SslEnabled ssl, size_t maxPayload,
    const Proc& onReceive, F0 lifetimeTransfo = F0{}, F1 syncTransfo = F1{});

  //template<typename N, typename S, typename M, typename Proc>
  //void receiveMessage(const S& socket, M&& ptrMsg, SslEnabled&& ssl, size_t maxPayload, const Proc& onReceive);

  namespace detail
  {
    /// Network N,
    /// Mutable<SslSocket<N>> S,
    /// Mutable<Message> M,
    /// Procedure<Optional<M> (ErrorCode<N>, M)> Proc,
    /// Transformation<Procedure> F0,
    /// Transformation<Procedure<void (Args...)>> F1
    template<typename N, typename S, typename M, typename Proc, typename F0, typename F1>
    void onReadData(const ErrorCode<N>& erc, const S& socket, M ptrMsg, SslEnabled ssl,
      size_t maxPayload, Proc onReceive, const F0& lifetimeTransfo, const F1& syncTransfo)
    {
      if (auto optionalPtrNextMsg = onReceive(erc, ptrMsg))
      {
        receiveMessage<N>(socket, *optionalPtrNextMsg, ssl, maxPayload, onReceive, lifetimeTransfo, syncTransfo);
      }
    }

    /// Performs various checks against the received message header, then starts
    /// an asynchronous read to get the message data.
    ///
    /// Note: The message size must not exceed the given maximum payload.
    ///
    /// Network N,
    /// Mutable<SslSocket<N>> S,
    /// Mutable<Message> M,
    /// Procedure<Optional<M> (ErrorCode<N>, M)> Proc,
    /// Transformation<Procedure> F0,
    /// Transformation<Procedure<void (Args...)>> F1
    template<typename N, typename S, typename M, typename Proc, typename F0, typename F1>
    void onReadHeader(const ErrorCode<N>& erc, std::size_t len,
      const S& socket, M ptrMsg, SslEnabled ssl,
      size_t maxPayload, Proc onReceive, F0 lifetimeTransfo, F1 syncTransfo)
    {
      auto receiveErrorAndMaybeReceiveNext = [&](ErrorCode<N> erc) {
        if (auto optionalPtrMsg = onReceive(erc, M{}))
        {
          receiveMessage<N>(socket, *optionalPtrMsg, ssl, maxPayload, onReceive,
            lifetimeTransfo, syncTransfo);
        }
      };
      if (erc)
      {
        receiveErrorAndMaybeReceiveNext(erc);
        return;
      }
      // When using SSL, sometimes we are called spuriously.
      // Consider this only in SSL ?
      if (*ssl && len == 0)
      {
        receiveMessage<N>(socket, ptrMsg, ssl, maxPayload, onReceive,
          lifetimeTransfo, syncTransfo);
        return;
      }
      auto& msg = *ptrMsg;
      auto header = msg.header();
      if (header.magic != Message::Header::magicCookie)
      {
        qiLogWarning(logCategory()) << &(*socket) << ": Incorrect magic from "
          << (*socket).lowest_layer().remote_endpoint().address().to_string()
          << " (expected " << Message::Header::magicCookie
          << ", got " << header.magic << ").";
        receiveErrorAndMaybeReceiveNext(fault<ErrorCode<N>>());
        return;
      }
      size_t payload = header.size;
      if (payload == 0u)
      {
        onReadData<N>(success<ErrorCode<N>>(), socket, ptrMsg, ssl, maxPayload,
          onReceive, lifetimeTransfo, syncTransfo);
        return;
      }
      if (payload > maxPayload)
      {
        qiLogWarning(logCategory()) << "Receiving message of size " << payload
          << " above maximum configured payload size " << maxPayload <<
             " (configure with environment variable QI_MAX_MESSAGE_PAYLOAD).";
        receiveErrorAndMaybeReceiveNext(messageSize<ErrorCode<N>>());
        return;
      }
      auto messageBuffer = msg.extractBuffer();
      void* ptr = messageBuffer.reserve(payload);
      auto buffer = N::buffer(ptr, payload);
      msg.setBuffer(std::move(messageBuffer));
      auto readData = lifetimeTransfo([=](ErrorCode<N> error, std::size_t /*len*/) {
        onReadData<N>(error, socket, ptrMsg, ssl, maxPayload, onReceive, lifetimeTransfo, syncTransfo);
      });
      if (*ssl)
      {
        N::async_read(*socket, buffer, syncTransfo(readData));
      }
      else
      {
        N::async_read((*socket).next_layer(), buffer, syncTransfo(readData));
      }
    }
  } // namespace detail

  /// Network N,
  /// Mutable<SslSocket<N>> S,
  /// Mutable<Message> M,
  /// Procedure<Optional<M> (ErrorCode<N>, M)> Proc,
  /// Transformation<Procedure> F0,
  /// Transformation<Procedure<void (Args...)>> F1
  template<typename N, typename S, typename M, typename Proc, typename F0, typename F1>
  void receiveMessage(const S& socket, M ptrMsg, SslEnabled ssl, size_t maxPayload,
      const Proc& onReceive, F0 lifetimeTransfo, F1 syncTransfo)
  {
    auto makeHeaderBuffer = [&] {
      return N::buffer(&ptrMsg->header(), sizeof(Message::Header));
    };
    auto readHeader = syncTransfo(lifetimeTransfo([=](ErrorCode<N> erc, std::size_t len) {
      detail::onReadHeader<N>(erc, len, socket, ptrMsg, ssl, maxPayload, onReceive,
        lifetimeTransfo, syncTransfo);
    }));
    if (*ssl)
    {
      N::async_read(*socket, makeHeaderBuffer(), readHeader);
    }
    else
    {
      N::async_read((*socket).next_layer(), makeHeaderBuffer(), readHeader);
    }
  }

  /// Receive continuously messages until told to stop.
  ///
  /// A handler is called when a message is received.
  /// The handler can return `false` to stop the message receiving.
  /// The Message pointer passed to the handler is only valid if there is no
  /// error (the error code is false).
  ///
  /// The only role of this type is to store a message.
  /// The message receiving is handled by `receiveMessage`.
  ///
  /// Warning: The instance must remain alive until the handler is called.
  /// You can provide a procedure transformation (`lifetimeTransfo`) that will
  /// wrap the handler and handle the expired instance case.
  /// `ReceiveMessageContinuousTrack` does this for you by relying on `Trackable`.
  ///
  /// A sync procedure transformation can also be provided to wrap any
  /// handler passed to the network. A typical use is to strand the handler.
  ///
  /// Example: receiving messages until an error occurs
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// ReceiveMessageContinuous<N> c{socket, SslEnabled{false}, maxPayload,
  ///   [=](Error e, const Message* msgPtr) mutable {
  ///     if (e) {
  ///       // treat error
  ///       return false; // Stop receiving messages.
  ///     }
  ///     // use msgPtr
  ///     return true; // Continue receiving messages.
  ///   }};
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// Network N
  template<typename N>
  class ReceiveMessageContinuous
  {
    Message _msg;
  public:
  // QuasiRegular:
    ReceiveMessageContinuous() = default;
    // TODO: uncomment when messages are comparable, or when latest GCC is fixed.
//    QI_GENERATE_FRIEND_REGULAR_OPS_1(ReceiveMessageContinuous, _msg)
  // Procedure:
    /// Mutable<SslSocket<N>> S,
    /// Procedure<bool (ErrorCode<N>, const Message*)> Proc,
    /// Transformation<Procedure> F0,
    /// Transformation<Procedure<void (Args...)>> F1
    template<typename S, typename Proc, typename F0 = IdTransfo, typename F1 = IdTransfo>
    void operator()(const S& socket, SslEnabled ssl, size_t maxPayload,
        Proc onReceive, const F0& lifetimeTransfo = {}, const F1& syncTransfo = {})
    {
      receiveMessage<N>(socket, &_msg, ssl, maxPayload,
        [=](ErrorCode<N> erc, const Message* m) mutable -> boost::optional<Message*> {
          if (onReceive(erc, m))
          {
            // Must continue.
            auto dataBuffer = _msg.extractBuffer();
            dataBuffer.clear();
            _msg.setBuffer(std::move(dataBuffer));
            return {&_msg};
          }
          return {};
        },
        lifetimeTransfo,
        syncTransfo
      );
    }
  };

  /// Receive continuously messages until told to stop and track the object's
  /// lifetime.
  ///
  /// The only difference with `ReceiverMessageContinuous` is that
  /// if the instance is destroyed before the callback is called,
  /// the callback will be called with a `operation aborted` error.
  ///
  /// Network N
  template<typename N>
  class ReceiveMessageContinuousTrack : public Trackable<ReceiveMessageContinuousTrack<N>>
  {
    using Trackable<ReceiveMessageContinuousTrack>::destroy;
    ReceiveMessageContinuous<N> _receiveMsg;
  public:
  // QuasiRegular:
    ReceiveMessageContinuousTrack() = default;
    QI_GENERATE_FRIEND_REGULAR_OPS_1(ReceiveMessageContinuousTrack, _receiveMsg)
  // Procedure:
    /// Mutable<<SslSocket<N>> S,
    /// Procedure<bool (ErrorCode<N>, const Message*)> Proc
    /// Transformation<Procedure<void (Args...)>> F
    template<typename S, typename Proc, typename F = IdTransfo>
    void operator()(const S& socket, SslEnabled ssl, size_t maxPayload,
      Proc onReceive, const F& syncTransfo = {})
    {
      auto lifetimeTransfo = trackWithFallbackTransfo(
        [=]() mutable {
          onReceive(operationAborted<ErrorCode<N>>(), nullptr);
        },
        this);

      _receiveMsg(socket, ssl, maxPayload, onReceive, lifetimeTransfo, syncTransfo);
    }
    ~ReceiveMessageContinuousTrack()
    {
      destroy();
    }
  };
}} // namespace qi::sock

#endif // _QI_SOCK_RECEIVE_HPP
