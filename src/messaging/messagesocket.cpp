#include <qi/log.hpp>
#include <src/messaging/sock/option.hpp>
#include "messagesocket.hpp"
#include "tcpmessagesocket.hpp"

// Disable "'this': used in base member initializer list"
#if BOOST_COMP_MSVC
# pragma warning(push)
# pragma warning(disable: 4355)
#endif


qiLogCategory(qi::sock::logCategory());

namespace qi
{
  MessageSocket::~MessageSocket()
  {
    qiLogDebug() << "Destroying transport socket";
    _signalsStrand.join();
  }

  bool MessageSocket::isConnected() const
  {
    return status() == qi::MessageSocket::Status::Connected;
  }

  MessageSocketPtr makeMessageSocket(const std::string &protocol, qi::EventLoop *eventLoop)
  {
    return makeTcpMessageSocket(protocol, eventLoop);
  }

  MessageDispatchConnection::MessageDispatchConnection() noexcept = default;

  MessageDispatchConnection::MessageDispatchConnection(MessageSocketPtr socket,
                                                       MessageDispatcher::RecipientId recipientId,
                                                       MessageDispatcher::MessageHandler handler)
    : _socket(socket)
    , _recipientId(recipientId)
    , _messageDispatcherLink(
        socket ?
          socket->messagePendingConnect(_recipientId.serviceId, _recipientId.objectId, handler) :
          throw std::invalid_argument(
            "Cannot connect handler to socket message dispatch: the socket pointer is null."))
  {
  }

  MessageDispatchConnection::MessageDispatchConnection(MessageDispatchConnection&& other)
    : _socket(ka::exchange(other._socket, {}))
    , _recipientId(ka::exchange(other._recipientId, defaultRecipientId()))
    , _messageDispatcherLink(
        ka::exchange(other._messageDispatcherLink, SignalBase::invalidSignalLink))

  {
  }

  MessageDispatchConnection& MessageDispatchConnection::operator=(MessageDispatchConnection&& other)
  {
    if (&other == this)
      return *this;

    reset();
    _socket = ka::exchange(other._socket, {});
    _recipientId = ka::exchange(other._recipientId, defaultRecipientId());
    _messageDispatcherLink =
      ka::exchange(other._messageDispatcherLink, SignalBase::invalidSignalLink);
    return *this;
  }

  MessageDispatchConnection::~MessageDispatchConnection()
  {
    reset();
  }

  void MessageDispatchConnection::reset()
  {
    if (_messageDispatcherLink == SignalBase::invalidSignalLink)
      return;
    if (auto sock = socket())
      sock->messagePendingDisconnect(_recipientId.serviceId, _recipientId.objectId,
                                     _messageDispatcherLink);
  }

  MessageDispatcher::RecipientId MessageDispatchConnection::defaultRecipientId() noexcept
  {
    return { Message::Service_Server, Message::GenericObject_None };
  }

}

#if BOOST_COMP_MSVC
# pragma warning(pop)
#endif
