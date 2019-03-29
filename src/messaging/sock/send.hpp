#pragma once
#ifndef _QI_SOCK_SEND_HPP
#define _QI_SOCK_SEND_HPP
#include <atomic>
#include <vector>
#include <list>
#include <stdexcept>
#include <sstream>
#include <boost/thread/synchronized_value.hpp>
#include <boost/core/ignore_unused.hpp>
#include <ka/src.hpp>
#include <ka/scoped.hpp>
#include <qi/trackable.hpp>
#include <qi/future.hpp>
#include <qi/atomic.hpp>
#include "src/messaging/message.hpp"
#include "concept.hpp"
#include "traits.hpp"
#include "option.hpp"
#include "error.hpp"
#include "common.hpp"


/// @file
/// Contains functions and types related to message sending on a socket.
///
/// # General overview
///
/// ## The message send loop
///
/// The most fundamental function in this file, `sendMessage`, implements a
/// message send loop. It is responsible for calling lower layer network
/// API and for providing upper layer the sent message through a callback.
/// Its implementation can be pictured by the following diagram (some error
/// handling is omitted):
///
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///           start
///             |
///             v
///      async write msg <--------------
///             |                       |
///             v                       |
/// pass msg/error to upper layer*      |
///             |                       |
///       must continue? ---------------
///             | no         yes
///             v
///            stop
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///
/// * The upper layer is passed, via a callback, the sent message and the error
/// if one occurred, so that it can decide if the message sending must continue.
/// The upper layer does so by returning a "pointer" to the next message to
/// send, or nothing if sending must stop (this is done by using an optional
/// pointer).
///
/// ## Lifetime and synchronization mechanisms
///
/// See the same section in `receive.hpp`.
///
///
/// ## Utility components
///
/// The memory for the messages can be for example maintained by an instance of
/// `SendMessageEnqueue`. As `sendMessage`, it implements a message
/// send loop, but being an object it can have a state and takes leverage
/// of this to maintain a message queue. It passes the first message of the queue to
/// `sendMessage` and removes it from the queue when sending is done. In this
/// case, `SendMessageEnqueue` effectively constitutes the upper layer of
/// `sendMessage`.
///
/// `SendMessageEnqueue` has itself an upper layer: it passes it the
/// sent message though a callback. This callback returns a boolean to
/// signal if message sending must continue.
///
/// `SendMessageEnqueue`'s kinematics is pictured in the following diagram
/// (`_msgQueue` denotes its message queue member):
///
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///  SendMessageEnqueue start
///             |
///             v
///   sendMessage(_msgQueue.begin()) <--
///             | message sent          |
///             v                       |
/// pass msg/error to upper layer*      |
///             |                       |
///             v                       |
///   remove msg from queue             |
///             |                       |
///       must continue? ---------------
///             | no         yes
///             v
///            stop
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///
///
/// ## Data exchange through layers
///
/// Finally, this is how data is exchanged through callbacks between the
/// different layers:
///
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Layer 2:                ...
///                         ^ | bool
///         (Error, IterMsg)| v
/// Layer 1:         SendMessageEnqueue
///                         ^ | optional<IterMsg>
///         (Error, IterMsg)| v
/// Layer 0:            sendMessage
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

namespace qi { namespace sock {

  /// Make network buffers for the given message.
  ///
  /// One buffer is for the header and the other one is for data.
  ///
  /// Network N
  template<typename N>
  std::vector<ConstBuffer<N>> makeBuffers(const Message& msg)
  {
    // header buffer
    ConstBuffer<N> headerBuffer = N::buffer(static_cast<const void*>(&msg.header()),
      sizeof(Message::Header));
    std::vector<ConstBuffer<N>> buffers;
    const auto& msgBuffer = msg.buffer();

    // A buffer has a header and data.
    // Inside data, subbuffers' sizes are interleaved.
    // Subbuffers are _not_ inside the buffer, but in separate memory.
    // We're going to send to the network:
    // - the header
    // - for all subbuffers:
    //  - the data chunk in the main buffer up to (and including) the subbuffer's size
    //  - the subbuffer
    // - the last data chunk in the main buffer
    //
    // Memory layout for a buffer with 2 subbuffers:
    // (low address)                                                         (high address)
    // |header|buffer_part_0|size_subbuffer_0|buffer_part_1|size_subbuffer_1|buffer_part_2|
    buffers.reserve(1 + 2 * msgBuffer.subBuffers().size() + 1);
    buffers.push_back(headerBuffer);

    decltype(msgBuffer.size()) beginOffset = 0;
    // subbuffers
    for (const auto& sub: msgBuffer.subBuffers())
    {
      // buffer chunk between startOffset and the offset past the next subbuffer's size
      const auto sizeOffset = sub.first;
      auto endOffset = sizeOffset + sizeof(Buffer::size_type);
      if (endOffset != beginOffset)
        buffers.push_back(N::buffer(
          static_cast<const char*>(msgBuffer.data()) + beginOffset, endOffset - beginOffset));
      beginOffset = endOffset;
      // subbuffer
      const auto& subBuffer = sub.second;
      buffers.push_back(N::buffer(subBuffer.data(), subBuffer.size()));
    }
    // end of main buffer
    buffers.push_back(N::buffer(
      static_cast<const char*>(msgBuffer.data()) + beginOffset, msgBuffer.size() - beginOffset));
    return buffers;
  }

  /// Send a message through the socket and call the handler when the operation
  /// is complete, successfully or not.
  ///
  /// If the handler returns a new message, it is immediately sent.
  ///
  /// Precondition: The message referred to by `cptrMsg` must be valid until the
  ///   handler has been called.
  ///
  /// Precondition: This function must not be called while a message is already
  ///   being sent. It is possible to call it again only once the handler as
  ///   been called.
  ///
  /// Example:
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// // `It` is the iterator type of the message queue.
  /// // Precondition: msgQueue is not empty.
  /// sendMessage<N>(socket, msgQueue.begin(),
  ///   [](Error e, It itSentMsg) {
  ///     // Check error...
  ///     msgQueue.erase(itSentMsg);
  ///     boost::optional<It> itNext;
  ///     if (!msgQueue.empty()) itNext = msgQueue.begin();
  ///     return itNext;
  ///   },
  ///   OptionSsl{false});
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// Network N,
  /// Mutable<SslSocket<N>> S,
  /// Readable<Message> M,
  /// Procedure<Optional<M> (ErrorCode<N>, M)> Proc,
  /// Transformation<Procedure> F0,
  /// Transformation<Procedure<void (Args...)>> F1
  template<typename N, typename S, typename M, typename Proc, typename F0 = ka::id_transfo_t, typename F1 = ka::id_transfo_t>
  void sendMessage(const S& socket, M cptrMsg, Proc onSent, SslEnabled ssl,
      F0 lifetimeTransfo = {}, F1 syncTransfo = {})
  {
    auto buffers = makeBuffers<N>(*cptrMsg);
    auto writeCont = syncTransfo(lifetimeTransfo([=](ErrorCode<N> erc, size_t /*len*/) mutable {
      if (auto optionalCptrNextMsg = onSent(erc, cptrMsg))
      {
        sendMessage<N>(socket, *optionalCptrNextMsg, onSent, ssl, lifetimeTransfo, syncTransfo);
      }
    }));
    if (*ssl)
    {
      N::async_write(*socket, std::move(buffers), writeCont);
    }
    else
    {
      N::async_write((*socket).next_layer(), std::move(buffers), writeCont);
    }
  }

  /// Functor that sends messages through a socket.
  ///
  /// The role of this type is to provide a queue for messages.
  /// You can therefore ask to send a message before the current one has
  /// actually been sent. The message will simply be enqueued and sent ASAP.
  /// The messages will be sent in a FIFO manner.
  /// Sending messages is thread-safe.
  ///
  /// The actual sending is done by `sendMessage`.
  ///
  /// When a message has been sent, a callback is called. This callback return
  /// a boolean to decide if the queue, if not empty, must continue to be processed.
  ///
  /// If you decide to stop the queue processing and it contain some messages,
  /// the queue is not cleared. Next time you send a message, it will
  /// be enqueued and the queue processing will continue from where it had stopped.
  ///
  /// Warning: The instance must remain alive until messages are sent.
  /// You can provide a procedure transformation (`lifetimeTransfo`) that will
  /// wrap any internal callback and handle the expired instance case.
  /// `SendMessageEnqueueTrack` does this for you by relying on `Trackable`.
  ///
  /// A sync procedure transformation can also be provided to wrap any
  /// callback passed to the network. A typical use is to strand the callback.
  ///
  /// Network N, Mutable<NetSslSocket> S
  template<typename N, typename S>
  struct SendMessageEnqueue
  {
    using ReadableMessage = std::list<Message>::const_iterator;
    SendMessageEnqueue()
      : _sending{false}
    {
    }
    explicit SendMessageEnqueue(const S& socket)
      : _socket(socket)
      , _sending{false}
    {
    }
  // Procedure:
    /// Message Msg,
    /// Procedure<bool (ErrorCode<N>, Readable<Message>)> Proc,
    /// Transformation<Procedure> F0,
    /// Transformation<Procedure<void (Args...)>> F1
    template<typename Msg,
             typename Proc = ka::constant_function_t<bool>,
             typename F0 = ka::id_transfo_t, typename F1 = ka::id_transfo_t>
    void operator()(Msg&&, SslEnabled, Proc onSent = Proc{true},
      const F0& lifetimeTransfo = F0{}, const F1& syncTransfo = F1{});
  private:
    S _socket;
    /// A list is used because we need the iterators not to be invalidated by
    /// insertions at begin or end, which is not the case with deque.
    /// See [23.3.3.4 deque modifiers].
    std::list<Message> _sendQueue;
    bool _sending;
    std::mutex _sendMutex;
  };

  // Lemma SendMessageEnqueue.0:
  //  If a message is already being sent, the message is queued without
  //  invalidating the one being sent.
  // Proof:
  //  All messages are put in the send queue, including the one being sent.
  //  The send queue is a list so adding an element doesn't invalidate the other ones.
  template<typename N, typename S>
  template<typename Msg, typename Proc, typename F0, typename F1>
  void SendMessageEnqueue<N, S>::operator()(Msg&& msg, SslEnabled ssl, Proc onSent,
      const F0& lifetimeTransfo, const F1& syncTransfo)
  {
    qiLogDebug(logCategory()) << _socket.get() << " SendMessageEnqueue()(" << msg.type() << ": " << msg.address() << ", ssl=" << *ssl << ")";
    using I = decltype(_sendQueue.begin());
    I itMsg;
    bool mustStartSendLoop = false;
    {
      std::lock_guard<std::mutex> lock{_sendMutex};
      _sendQueue.emplace_back(std::forward<Msg>(msg));
      itMsg = _sendQueue.begin();
      // We've just added a message to the queue, so if we are not currently sending,
      // we must (re)start the send loop.
      if (!_sending)
      {
        _sending = true;
        mustStartSendLoop = true;
      }
    }
    if (mustStartSendLoop)
    {
      // Lemma SendMessageEnqueue.1:
      //  When calling sendMessage, itMsg is still valid.
      // Proof:
      //  The send queue is a std::list, so inserting or erasing other elements
      //  doesn't invalidate the iterator.
      //  Each thread adds a message to the send queue. But only one at a time
      //  can enter this branch (by tryRaiseAtomicFlag.0).
      //  Also, the sending flag is only modified while the queue is locked, so
      //  the scenario where a thread B adds a message to the queue, is suspended
      //  just before evaluating the condition of this branch, then the send loop
      //  thread A clears the queue, and then the thread B resumes, is correctly handled.
      //  Moreover, this branch results in exactly one message being removed from
      //  the send queue (by SendMessageEnqueue.2).
      //  Therefore, at this point the number of messages in the send queue is
      //  always at least 1.

      // Lemma SendMessageEnqueue.2:
      //  eraseAndReturnNextMessage erases from the send queue the element pointed
      //  by the given iterator, even if an exception is thrown.

      // This callback will be called when a message has been sent, or an error
      // occurred. It passes an iterator on the sent message to the upper layer,
      // which in return decides whether sending of the enqueued messaged must
      // continue. Then, the callback erase the message.
      auto eraseAndReturnNextMessage =
        [&, onSent](ErrorCode<N> erc, I itSent) mutable -> boost::optional<I> {
          // It's ok to allow new sendings once the current one is complete.
          bool mustContinue = false;
          boost::optional<I> itNext;
          try
          {
            // A scoped is used to cope with potential exception thrown by onSent.
            auto scopedErase = ka::scoped([&] {
              std::lock_guard<std::mutex> lock{_sendMutex};
              _sendQueue.erase(itSent);
              if (!mustContinue || _sendQueue.empty())
              {
                QI_ASSERT(_sending);
                if (!_sending)
                  qiLogWarning(logCategory()) << "SendMessageEnqueue: sending flag should be raised.";
                _sending = false;
                return;
              }
              itNext = _sendQueue.begin();
            });
            mustContinue = onSent(erc, itSent);
          }
          catch (const std::exception& e)
          {
            qiLogError(logCategory()) << "Error in post-send phase: " << e.what();
            throw;
          }
          return itNext;
        };

      sendMessage<N>(_socket, itMsg, std::move(eraseAndReturnNextMessage), ssl,
        lifetimeTransfo, syncTransfo);
    }
  }

  /// Functor that sends messages and tracks the object's lifetime.
  ///
  /// The only difference with `SendMessageEnqueue` is that with this type, if
  /// the instance is destroyed before a internal callback is called,
  /// the callback will be called with a `operation aborted` error.
  ///
  /// Network N
  template<typename N, typename S>
  struct SendMessageEnqueueTrack : Trackable<SendMessageEnqueueTrack<N, S>>
  {
    using ReadableMessage = typename SendMessageEnqueue<N, S>::ReadableMessage;
    using Trackable<SendMessageEnqueueTrack>::destroy;

    SendMessageEnqueueTrack() = default;
    explicit SendMessageEnqueueTrack(const S& socket)
      : _sendMsg{socket}
    {
    }
    ~SendMessageEnqueueTrack()
    {
      destroy();
    }
  // Procedure:
    /// Message Msg, Procedure<void (ErrorCode<N>, Readable<Message>)> Proc, Transformation<Procedure<void (Args...)>> F
    template<typename Msg, typename Proc = ka::constant_function_t<void>, typename F = ka::id_transfo_t>
    void operator()(Msg&& m, SslEnabled ssl, Proc onSent = Proc{}, F syncTransfo = F{})
    {
      auto lifetimeTransfo = trackWithFallbackTransfo([=]() mutable {
          onSent(operationAborted<ErrorCode<N>>(), {});
        },
        this
      );
      _sendMsg(std::forward<Msg>(m), ssl, onSent, lifetimeTransfo, syncTransfo);
    }
  private:
    SendMessageEnqueue<N, S> _sendMsg;
  };
}} // namespace qi::sock

#endif // _QI_SOCK_SEND_HPP
