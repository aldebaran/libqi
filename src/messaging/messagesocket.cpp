#include <qi/log.hpp>
#include <src/messaging/sock/option.hpp>
#include "messagesocket.hpp"
#include "tcpmessagesocket.hpp"

// Disable "'this': used in base member initializer list"
#include <ka/macro.hpp>
KA_WARNING_PUSH()
KA_WARNING_DISABLE(4355, )

qiLogCategory(qi::sock::logCategory());

namespace qi
{
  MessageSocket::~MessageSocket()
  {
    QI_LOG_DEBUG_SOCKET(this) << "Destroying transport socket";
    _signalsStrand.join();
  }

  bool MessageSocket::isConnected() const
  {
    return status() == qi::MessageSocket::Status::Connected;
  }

  MessageSocketPtr makeMessageSocket(ssl::ClientConfig sslConfig, qi::EventLoop* eventLoop)
  {
    return makeTcpMessageSocket(std::move(sslConfig), eventLoop);
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
    , _messageDispatcherLink(exchangeInvalidSignalLink(other._messageDispatcherLink))

  {
  }

  MessageDispatchConnection& MessageDispatchConnection::operator=(MessageDispatchConnection&& other)
  {
    if (&other == this)
      return *this;

    reset();
    _socket = ka::exchange(other._socket, {});
    _recipientId = ka::exchange(other._recipientId, defaultRecipientId());
    _messageDispatcherLink = exchangeInvalidSignalLink(other._messageDispatcherLink);
    return *this;
  }

  MessageDispatchConnection::~MessageDispatchConnection()
  {
    reset();
  }

  void MessageDispatchConnection::reset()
  {
    const auto link = exchangeInvalidSignalLink(_messageDispatcherLink);
    if (!isValidSignalLink(link))
      return;
    if (auto sock = socket())
      sock->messagePendingDisconnect(_recipientId.serviceId, _recipientId.objectId, link);
  }

  MessageDispatcher::RecipientId MessageDispatchConnection::defaultRecipientId() noexcept
  {
    return { Message::Service_Server, Message::GenericObject_None };
  }

}

KA_WARNING_POP()
