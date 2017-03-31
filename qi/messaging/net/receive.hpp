#pragma once
#ifndef _QI_NET_RECEIVE_HPP
#define _QI_NET_RECEIVE_HPP
#include <qi/messaging/net/concept.hpp>
#include <qi/messaging/net/traits.hpp>
#include <qi/messaging/net/option.hpp>
#include <qi/messaging/net/error.hpp>
#include <qi/messaging/net/common.hpp>
#include "src/messaging/message.hpp"
#include <qi/trackable.hpp>
#include <qi/log.hpp>
#include <qi/macroregular.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/optional.hpp>

/// @file
/// Contains functions and types related to message reception on a socket.

namespace qi { namespace net {
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
    const Proc& onReceive, F0 dataTransfo = F0{}, F1 netTransfo = F1{});

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
      size_t maxPayload, Proc onReceive, const F0& dataTransfo, const F1& netTransfo)
    {
      if (auto optionalPtrNextMsg = onReceive(erc, ptrMsg))
      {
        receiveMessage<N>(socket, *optionalPtrNextMsg, ssl, maxPayload, onReceive, dataTransfo, netTransfo);
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
      size_t maxPayload, Proc onReceive, F0 dataTransfo, F1 netTransfo)
    {
      auto receiveErrorAndMaybeReceiveNext = [&](ErrorCode<N> erc) {
        if (auto optionalPtrMsg = onReceive(erc, M{}))
        {
          receiveMessage<N>(socket, *optionalPtrMsg, ssl, maxPayload, onReceive,
            dataTransfo, netTransfo);
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
          dataTransfo, netTransfo);
        return;
      }
      auto& msg = *ptrMsg;
      auto header = msg._p->header;
      if (header.magic != MessagePrivate::magic)
      {
        qiLogWarning(logCategory()) << &(*socket) << ": Incorrect magic from "
          << (*socket).lowest_layer().remote_endpoint().address().to_string()
          << " (expected " << MessagePrivate::magic
          << ", got " << header.magic << ").";
        receiveErrorAndMaybeReceiveNext(fault<ErrorCode<N>>());
        return;
      }
      size_t payload = header.size;
      if (payload == 0u)
      {
        onReadData<N>(success<ErrorCode<N>>(), socket, ptrMsg, ssl, maxPayload,
          onReceive, dataTransfo, netTransfo);
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
      void* ptr = msg._p->buffer.reserve(payload);
      auto buffer = N::buffer(ptr, payload);
      auto readData = dataTransfo([=](ErrorCode<N> error, std::size_t /*len*/) {
        onReadData<N>(error, socket, ptrMsg, ssl, maxPayload, onReceive, dataTransfo, netTransfo);
      });
      if (*ssl)
      {
        N::async_read(*socket, buffer, netTransfo(readData));
      }
      else
      {
        N::async_read((*socket).next_layer(), buffer, netTransfo(readData));
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
      const Proc& onReceive, F0 dataTransfo, F1 netTransfo)
  {
    auto makeHeaderBuffer = [&] {
      return N::buffer((*ptrMsg)._p->getHeader(), sizeof(MessagePrivate::MessageHeader));
    };
    auto readHeader = dataTransfo([=](ErrorCode<N> erc, std::size_t len) {
      detail::onReadHeader<N>(erc, len, socket, ptrMsg, ssl, maxPayload, onReceive,
        dataTransfo, netTransfo);
    });
    if (*ssl)
    {
      N::async_read(*socket, makeHeaderBuffer(), netTransfo(readHeader));
    }
    else
    {
      N::async_read((*socket).next_layer(), makeHeaderBuffer(), netTransfo(readHeader));
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
  /// You can provide a procedure transformation (`dataTransfo`) that will
  /// wrap the handler and handle the expired instance case.
  /// `ReceiveMessageContinuousTrack` does this for you by relying on `Trackable`.
  ///
  /// A network procedure transformation can also be provided to wrap any
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
    QI_GENERATE_FRIEND_REGULAR_OPS_1(ReceiveMessageContinuous, _msg)
  // Procedure:
    /// Mutable<SslSocket<N>> S,
    /// Procedure<bool (ErrorCode<N>, const Message*)> Proc,
    /// Transformation<Procedure> F0,
    /// Transformation<Procedure<void (Args...)>> F1
    template<typename S, typename Proc, typename F0 = IdTransfo, typename F1 = IdTransfo>
    void operator()(const S& socket, SslEnabled ssl, size_t maxPayload,
        Proc onReceive, const F0& dataTransfo = {}, const F1& netTransfo = {})
    {
      receiveMessage<N>(socket, &_msg, ssl, maxPayload,
        [=](ErrorCode<N> erc, const Message* m) mutable -> boost::optional<Message*> {
          if (onReceive(erc, m))
          {
            // Must continue.
            _msg.buffer().clear();
            return {&_msg};
          }
          return {};
        },
        dataTransfo,
        netTransfo
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
      Proc onReceive, const F& netTransfo = {})
    {
      _receiveMsg(socket, ssl, maxPayload, onReceive,
        trackWithFallbackTransfo(
          [=]() mutable {
            onReceive(operationAborted<ErrorCode<N>>(), nullptr);
          },
          this),
        netTransfo);
    }
    ~ReceiveMessageContinuousTrack()
    {
      destroy();
    }
  };
}} // namespace qi::net

#endif // _QI_NET_RECEIVE_HPP
